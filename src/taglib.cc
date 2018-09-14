/*
 * Copyright (C) 2009 Nikhil Marathe ( nsm.nikhil@gmail.com )
 * This file is distributed under the MIT License. Please see
 * LICENSE for details
 */
#include "taglib.h"

#include <errno.h>
#include <string.h>

#include <v8.h>
#include <node_buffer.h>

#include <tfilestream.h>
#include <asffile.h>
#include <mpegfile.h>
#include <vorbisfile.h>
#include <flacfile.h>
#include <oggflacfile.h>
#include <mpcfile.h>
#include <mp4file.h>
#include <wavpackfile.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <aifffile.h>
#include <wavfile.h>
#include <apefile.h>
#include <modfile.h>
#include <s3mfile.h>
#include <itfile.h>
#include <xmfile.h>

#if _WINDOWS && WINVER >= 0x0602 // _WIN32_WINNT_WIN8 
#include <Processthreadsapi.h> // Windows 8 and Windows Server 2012
#elif _WINDOWS
#include <Windows.h> // older versions
#endif

#include "metadata.h"
#include "bufferstream.h"

using namespace v8;
using namespace node;
using namespace node_taglib;

namespace node_taglib {
int CreateFileRefPath(TagLib::FileName path, TagLib::FileRef **ref) {
    TagLib::FileRef *f = NULL;
    int error = 0;
    if (!TagLib::File::isReadable(path)) {
        error = EACCES;
    }
    else {
        f = new TagLib::FileRef(path);

        if ( f->isNull() || !f->tag() )
        {
            error = EINVAL;
            delete f;
            f = NULL;
        }
    }

    if (error != 0)
        *ref = NULL;
    else
        *ref = f;

    return error;
}

int CreateFileRef(TagLib::IOStream *stream, TagLib::String format, TagLib::FileRef **ref) {
    TagLib::FileRef *f = NULL;
    int error = 0;

    TagLib::File *file = createFile(stream, format);
    if (file == NULL) {
        *ref = NULL;
        return EBADF;
    }

    f = new TagLib::FileRef(file);

    if (f->isNull() || !f->tag() )
    {
        error = EINVAL;
        delete f;
        f = NULL;
    }

    if (error != 0)
        *ref = NULL;
    else
        *ref = f;

    return error;
}

TagLib::File *createFile(TagLib::IOStream *stream, TagLib::String format) {
    TagLib::File *file = NULL;
    format = format.upper();
    if (format == "MPEG")
        file = new TagLib::MPEG::File(stream, TagLib::ID3v2::FrameFactory::instance());
    else if (format == "OGG")
        file = new TagLib::Ogg::Vorbis::File(stream);
    else if (format == "OGG/FLAC")
        file = new TagLib::Ogg::FLAC::File(stream);
    else if (format == "FLAC")
        file = new TagLib::FLAC::File(stream, TagLib::ID3v2::FrameFactory::instance());
    else if (format == "MPC")
        file = new TagLib::MPC::File(stream);
    else if (format == "WV")
        file = new TagLib::WavPack::File(stream);
    else if (format == "SPX")
        file = new TagLib::Ogg::Speex::File(stream);
    else if (format == "TTA")
        file = new TagLib::TrueAudio::File(stream);
    else if (format == "MP4")
        file = new TagLib::MP4::File(stream);
    else if (format == "ASF")
        file = new TagLib::ASF::File(stream);
    else if (format == "AIFF")
        file = new TagLib::RIFF::AIFF::File(stream);
    else if (format == "WAV")
        file = new TagLib::RIFF::WAV::File(stream);
    else if (format == "APE")
        file = new TagLib::APE::File(stream);
    // module, nst and wow are possible but uncommon formatensions
    else if (format == "MOD")
        file = new TagLib::Mod::File(stream);
    else if (format == "S3M")
        file = new TagLib::S3M::File(stream);
    else if (format == "IT")
        file = new TagLib::IT::File(stream);
    else if (format == "XM")
        file = new TagLib::XM::File(stream);
    return file;
}

Local< String > ErrorToString(int error) {
    std::string err;

    switch (error) {
        case EACCES:
            err = "File is not readable";
            break;

        case EINVAL:
            err = "Failed to extract tags";
            break;

        case EBADF:
            err = "Unknown file format (check format string)";
            break;

        default:
            err = "Unknown error";
            break;
    }

    return Nan::New<String>(err).ToLocalChecked();
}


Local< Value > TagLibStringToString( TagLib::String s )
{
    if(s.isEmpty()) {
        return Nan::Null();
    }
    else {
        TagLib::ByteVector str = s.data(TagLib::String::UTF16);
        // Strip the Byte Order Mark of the input to avoid node adding a UTF-8
        // Byte Order Mark
        return Nan::New<v8::String>((uint16_t *)str.mid(2,str.size()-2).data(), s.size()).ToLocalChecked();
    }
}

TagLib::String NodeStringToTagLibString( Local<Value> s )
{
    if(s->IsNull()) {
        return TagLib::String::null;
    }
    else {
        Isolate* isolate = Isolate::GetCurrent();

        String::Utf8Value str(isolate, s->ToString());
        return TagLib::String(*str, TagLib::String::UTF8);
    }
}

void AddResolvers(const Nan::FunctionCallbackInfo<Value> &args)
{
    for (int i = 0; i < args.Length(); i++) {
        Local<Value> arg = args[i];
        if (arg->IsFunction()) {
            Local<Function> resolver = Local<Function>::Cast(arg);
            TagLib::FileRef::addFileTypeResolver(new CallbackResolver(resolver));
        }
    }
    args.GetReturnValue().SetUndefined();
}

CallbackResolver::CallbackResolver(Local< Function > func)
    : TagLib::FileRef::FileTypeResolver()
    , resolverFunc(func)
    // the constructor is always called in the v8 thread
#ifdef _WIN32
    , created_in(GetCurrentThreadId())
#else
    , created_in(pthread_self())
#endif
{
}

void CallbackResolver::invokeResolverCb(uv_async_t *handle)
{
    AsyncResolverBaton *baton = (AsyncResolverBaton *) handle->data;
    invokeResolver(baton);
    uv_async_send((uv_async_t*) &baton->idler);
    uv_close((uv_handle_t*)&baton->request, 0);
}

void CallbackResolver::stopIdling(uv_async_t *handle)
{
    uv_close((uv_handle_t*) handle, 0);
}

void CallbackResolver::invokeResolver(AsyncResolverBaton *baton)
{
    Nan::HandleScope scope;
    Handle<Value> argv[] = { TagLibStringToString(baton->fileName) };
    Local<Value> ret = Nan::Call(Nan::New(baton->resolver->resolverFunc), Nan::GetCurrentContext()->Global(), 1, argv).ToLocalChecked();
    if (!ret->IsString()) {
        baton->type = TagLib::String::null;
    }
    else {
        baton->type = NodeStringToTagLibString(ret->ToString()).upper();
    }
}

TagLib::File *CallbackResolver::createFile(TagLib::FileName fileName, bool readAudioProperties, TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const
{
    AsyncResolverBaton baton = AsyncResolverBaton();
    baton.request.data = (void *) &baton;
    baton.resolver = this;
#if _WINDOWS
    baton.fileName = fileName.toString();
#else
    baton.fileName = fileName;
#endif

#ifdef _WIN32
    if (created_in != GetCurrentThreadId()) {
#else
    if (created_in != pthread_self()) {
#endif
        uv_loop_t *wait_loop = uv_loop_new();
        uv_async_init(wait_loop, &baton.idler, CallbackResolver::stopIdling);

        uv_async_init(uv_default_loop(), &baton.request, invokeResolverCb);
        uv_async_send(&baton.request);
        uv_run(wait_loop, UV_RUN_DEFAULT);
        uv_loop_delete(wait_loop);
    }
    else {
        invokeResolver(&baton);
    }

    TagLib::FileStream *stream = new TagLib::FileStream(fileName);

    return node_taglib::createFile(stream, baton.type);
}
}

extern "C" {

static void
init (Handle<Object> target)
{
    Nan::HandleScope scope;

#ifdef TAGLIB_WITH_ASF
    Nan::Set(target, Nan::New("WITH_ASF").ToLocalChecked(), Nan::True());
#else
    Nan::Set(target, Nan::New("WITH_ASF").ToLocalChecked(), Nan::False());
#endif

#ifdef TAGLIB_WITH_MP4
    Nan::Set(target, Nan::New("WITH_MP4").ToLocalChecked(), Nan::True());
#else
    Nan::Set(target, Nan::New("WITH_MP4").ToLocalChecked(), Nan::False());
#endif

#ifdef ENABLE_RESOLVERS
    Nan::SetMethod(target, "addResolvers", AddResolvers);
#endif
    Metadata::Init(target);
}

NODE_MODULE(taglib, init)
}

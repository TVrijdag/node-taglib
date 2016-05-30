#include "metadata.h"

#include <errno.h>
#include <string.h>

#include <node_buffer.h>

#include "taglib.h"
#include "bufferstream.h"

using namespace node_taglib;
using namespace v8;
using namespace node;

namespace node_taglib {

NAN_MODULE_INIT(Metadata::Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>();
    tpl->SetClassName(Nan::New("Metadata").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
    Nan::SetPrototypeMethod(tpl, "save", AsyncSaveMetadata);
    Nan::SetPrototypeMethod(tpl, "saveSync", SyncSaveMetadata);
    Nan::SetPrototypeMethod(tpl, "close", CloseMetadata);
    
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("title").ToLocalChecked(), GetTitle, SetTitle);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("album").ToLocalChecked(), GetAlbum, SetAlbum);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("comment").ToLocalChecked(), GetComment, SetComment);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("artist").ToLocalChecked(), GetArtist, SetArtist);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("track").ToLocalChecked(), GetTrack, SetTrack);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("year").ToLocalChecked(), GetYear, SetYear);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("genre").ToLocalChecked(), GetGenre, SetGenre);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("length").ToLocalChecked(), GetAudioLength, NULL);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("bitrate").ToLocalChecked(), GetAudioBitrate, NULL);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("sampleRate").ToLocalChecked(), GetAudioSampleRate, NULL);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("channels").ToLocalChecked(), GetAudioChannels, NULL);
    
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    //Nan::Set(target, Nan::New("Metadata").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
    Nan::SetMethod(target, "read", AsyncReadMetadata);
    Nan::SetMethod(target, "readSync", SyncReadMetadata);
}

Metadata::Metadata(TagLib::FileRef * ffileRef) : tag(ffileRef->tag()), fileRef(ffileRef) { }

Metadata::~Metadata() {
    if (fileRef)
        delete fileRef;
    fileRef = NULL;
    tag = NULL;
}

inline Metadata * unwrapMetadata(const Nan::PropertyCallbackInfo<v8::Value>& info) {
  return Nan::ObjectWrap::Unwrap<Metadata>(info.Holder());
}
inline Metadata * unwrapMetadata(const Nan::PropertyCallbackInfo<void>& info) {
  return Nan::ObjectWrap::Unwrap<Metadata>(info.Holder());
}


NAN_GETTER(Metadata::GetTitle) {
    info.GetReturnValue().Set(TagLibStringToString(unwrapMetadata(info)->tag->title()));
}
NAN_SETTER(Metadata::SetTitle) {
    unwrapMetadata(info)->tag->setTitle(NodeStringToTagLibString(value));  
}

NAN_GETTER(Metadata::GetArtist) {
    info.GetReturnValue().Set(TagLibStringToString(unwrapMetadata(info)->tag->artist()));
}
NAN_SETTER(Metadata::SetArtist) {
    unwrapMetadata(info)->tag->setArtist(NodeStringToTagLibString(value));  
}

NAN_GETTER(Metadata::GetAlbum) {
    info.GetReturnValue().Set(TagLibStringToString(unwrapMetadata(info)->tag->album()));
}
NAN_SETTER(Metadata::SetAlbum) {
    unwrapMetadata(info)->tag->setAlbum(NodeStringToTagLibString(value));  
}

NAN_GETTER(Metadata::GetComment) {
    info.GetReturnValue().Set(TagLibStringToString(unwrapMetadata(info)->tag->comment()));
}
NAN_SETTER(Metadata::SetComment) {
    unwrapMetadata(info)->tag->setComment(NodeStringToTagLibString(value));  
}

NAN_GETTER(Metadata::GetTrack) {
    info.GetReturnValue().Set(unwrapMetadata(info)->tag->track());
}
NAN_SETTER(Metadata::SetTrack) {
    unwrapMetadata(info)->tag->setTrack(value->IntegerValue());  
}

NAN_GETTER(Metadata::GetYear) {
    info.GetReturnValue().Set(unwrapMetadata(info)->tag->year());
}
NAN_SETTER(Metadata::SetYear) {
    unwrapMetadata(info)->tag->setYear(value->IntegerValue());  
}

NAN_GETTER(Metadata::GetGenre) {
    info.GetReturnValue().Set(TagLibStringToString(unwrapMetadata(info)->tag->genre()));
}
NAN_SETTER(Metadata::SetGenre) {
    unwrapMetadata(info)->tag->setGenre(NodeStringToTagLibString(value));  
}



NAN_GETTER(Metadata::GetAudioLength) {
    TagLib::AudioProperties *props = unwrapMetadata(info)->fileRef->audioProperties();
    if (props) {
        info.GetReturnValue().Set(props->length());
    }
}

NAN_GETTER(Metadata::GetAudioBitrate) {
    TagLib::AudioProperties *props = unwrapMetadata(info)->fileRef->audioProperties();
    if (props) {
        info.GetReturnValue().Set(props->bitrate());
    }
}

NAN_GETTER(Metadata::GetAudioSampleRate) {
    TagLib::AudioProperties *props = unwrapMetadata(info)->fileRef->audioProperties();
    if (props) {
        info.GetReturnValue().Set(props->sampleRate());
    }
}

NAN_GETTER(Metadata::GetAudioChannels) {
    TagLib::AudioProperties *props = unwrapMetadata(info)->fileRef->audioProperties();
    if (props) {
        info.GetReturnValue().Set(props->channels());
    }
}



NAN_METHOD(Metadata::CloseMetadata) {
    Metadata *t = ObjectWrap::Unwrap<Metadata>(info.Holder());
    if (t->fileRef)
        delete t->fileRef;
    t->fileRef = NULL;
}

NAN_METHOD(Metadata::SyncSaveMetadata) {
    Metadata *m = ObjectWrap::Unwrap<Metadata>(info.Holder());

    // Check whether file descriptor is open
    if (m->fileRef == NULL) {
        Nan::ThrowError("Failed to save file: the file descriptor has already been closed");
        return;
    }

    assert(m->fileRef);
    bool success = m->fileRef->save();
    if (success)
        info.GetReturnValue().SetUndefined();
    else
        Nan::ThrowError("Failed to save file");
        //TODO: filename
}

NAN_METHOD(Metadata::AsyncSaveMetadata) {
    if (info.Length() >= 1 && !info[0]->IsFunction()) {
        Nan::ThrowError("Expected callback function as first argument");
        return;
    }

    Local<Function> callback = Local<Function>::Cast(info[0]);

    Metadata *m = ObjectWrap::Unwrap<Metadata>(info.Holder());

    // Check whether file descriptor is open
    if (m->fileRef == NULL) {
        Nan::ThrowError("Failed to save file: the file descriptor has already been closed");
        return;
    }

    AsyncBaton *baton = new AsyncBaton();
    baton->request.data = baton;
    baton->metadata = m;
    baton->callback.Reset(callback);
    baton->error = 1;

    uv_queue_work(uv_default_loop(), &baton->request, Metadata::AsyncSaveMetadataDo, (uv_after_work_cb)Metadata::AsyncSaveMetadataAfter);

    info.GetReturnValue().SetUndefined();
}

void Metadata::AsyncSaveMetadataDo(uv_work_t *req) {
    AsyncBaton *baton = static_cast<AsyncBaton*>(req->data);

    //assert(baton->tag->fileRef);
    if (baton->metadata->fileRef == NULL)
        baton->error = true;
    else
        baton->error = !baton->metadata->fileRef->save();
}

void Metadata::AsyncSaveMetadataAfter(uv_work_t *req) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    AsyncBaton *baton = static_cast<AsyncBaton*>(req->data);

    if (baton->error) {
        Local<Object> error = Nan::New<Object>();
        // Check whether the file descriptor was closed
        if (baton->metadata->fileRef == NULL) {
            error->Set(Nan::New("message").ToLocalChecked(), Nan::New("Failed to save file: the file descriptor has already been closed").ToLocalChecked());
        }
        else {
            error->Set(Nan::New("message").ToLocalChecked(), Nan::New("Failed to save file").ToLocalChecked());
            error->Set(Nan::New("path").ToLocalChecked(), Nan::New(baton->metadata->fileRef->file()->name()).ToLocalChecked());
        }
        Handle<Value> argv[] = { error };
        Nan::Call(Nan::New(baton->callback), Nan::GetCurrentContext()->Global(), 1, argv);
    }
    else {
        Handle<Value> argv[] = { Nan::Null() };
        Nan::Call(Nan::New(baton->callback), Nan::GetCurrentContext()->Global(), 1, argv);
    }

    baton->callback.Reset();
    delete baton;
}

NAN_METHOD(Metadata::SyncReadMetadata) {
    TagLib::FileRef *f = 0;
    int error = 0;

    if (info.Length() >= 1 && info[0]->IsString()) {
        String::Utf8Value path(info[0]->ToString());
        if ((error = CreateFileRefPath(*path, &f))) {
            Local<String> fn = String::Concat(info[0]->ToString(), Nan::New(": ", -1).ToLocalChecked());
            Nan::ThrowError(String::Concat(fn, ErrorToString(error)));
            return;
        }
    } else if (info.Length() >= 1 && Buffer::HasInstance(info[0])) {
        if (info.Length() < 2 || !info[1]->IsString()) {
            Nan::ThrowError("Expected string 'format' as second argument");
            return;
        }

        if ((error = CreateFileRef(new BufferStream(info[0]->ToObject()), NodeStringToTagLibString(info[1]->ToString()), &f))) {
            Nan::ThrowError(ErrorToString(error));
            return;
        }
    } else {
        Nan::ThrowError("Expected string or buffer as first argument");
        return;
    }

    Metadata * metadata = new Metadata(f);
    Local<Object> inst = Nan::NewInstance(Nan::New(constructor())).ToLocalChecked();
    metadata->Wrap(inst);

    info.GetReturnValue().Set(inst);
}

NAN_METHOD(Metadata::AsyncReadMetadata) {
    if (info.Length() < 1) {
        Nan::ThrowError("Expected string or buffer as first argument");
        return;
    }

    if (info[0]->IsString()) {
        if (info.Length() < 2 || !info[1]->IsFunction()) {
            Nan::ThrowError("Expected callback function as second argument");
            return;
        }
    } else if (Buffer::HasInstance(info[0])) {
        if (info.Length() < 2 || !info[1]->IsString()) {
            Nan::ThrowError("Expected string 'format' as second argument");
            return;
        }
        if (info.Length() < 3 || !info[2]->IsFunction()) {
            Nan::ThrowError("Expected callback function as third argument");
            return;
        }
    } else {
        Nan::ThrowError("Expected string or buffer as first argument");
        return;
    }


    AsyncBaton *baton = new AsyncBaton();
    baton->request.data = baton;
    baton->path = NULL;
    baton->metadata = NULL;
    baton->error = 0;

    if (info[0]->IsString()) {
        String::Utf8Value path(info[0]->ToString());
//#if _WINDOWS
//        baton->path = new TagLib::FileName(strdup(*path));
//#else
        baton->path = strdup(*path);
//#endif
        baton->callback.Reset(Local<Function>::Cast(info[1]));

    }
    else {
        baton->format = NodeStringToTagLibString(info[1]->ToString());
        baton->stream = new BufferStream(info[0]->ToObject());
        baton->callback.Reset(Local<Function>::Cast(info[2]));
    }

    uv_queue_work(uv_default_loop(), &baton->request, Metadata::AsyncMetadataReadDo, (uv_after_work_cb)Metadata::AsyncMetadataReadAfter);

    info.GetReturnValue().SetUndefined();
}

void Metadata::AsyncMetadataReadDo(uv_work_t *req) {
    AsyncBaton *baton = static_cast<AsyncBaton*>(req->data);

    TagLib::FileRef *f;

    if (baton->path) {
        baton->error = node_taglib::CreateFileRefPath(baton->path, &f);
    }
    else {
        assert(baton->stream);
        baton->error = node_taglib::CreateFileRef(baton->stream, baton->format, &f);
    }

    if (baton->error == 0) {
        baton->metadata = new Metadata(f);
    }
}

void Metadata::AsyncMetadataReadAfter(uv_work_t *req) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    AsyncBaton *baton = static_cast<AsyncBaton*>(req->data);

    if (baton->error) {
        Local<Object> error = Nan::New<Object>();
        error->Set(Nan::New("code").ToLocalChecked(), Nan::New(baton->error));
        error->Set(Nan::New("message").ToLocalChecked(), ErrorToString(baton->error));
        Handle<Value> argv[] = { error, Nan::Null() };
        Nan::Call(Nan::New(baton->callback), Nan::GetCurrentContext()->Global(), 2, argv);
    }
    else {
        Local<Object> inst = Nan::NewInstance(Nan::New(constructor())).ToLocalChecked();
        baton->metadata->Wrap(inst);
        Handle<Value> argv[] = { Nan::Null(), inst };
        Nan::Call(Nan::New(baton->callback), Nan::GetCurrentContext()->Global(), 2, argv);
    }

    baton->callback.Reset();
    delete baton->path;
    if (baton->path)
        delete baton->path;
    baton->path = NULL;
    if (baton->stream)
        delete baton->stream;
    baton->stream = NULL;
    delete baton;
    baton = NULL;
}
}

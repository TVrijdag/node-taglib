#ifndef NODE_TAGLIB_H
#define NODE_TAGLIB_H

#include <fileref.h>
#include <tag.h>
#include <node.h>
#include <uv.h>
#include <node_version.h>
#include <nan.h>

namespace node_taglib {
class Metadata;
class BufferStream;

/**
 * Note: This uses TagLib's internal file type resolvers
 * use CreateFileRef with a FileStream if the format is known
 */
int CreateFileRefPath(TagLib::FileName path, TagLib::FileRef **ref);
int CreateFileRef(TagLib::IOStream *stream, TagLib::String format, TagLib::FileRef **ref);
TagLib::File *createFile(TagLib::IOStream *stream, TagLib::String format);
v8::Local<v8::String> ErrorToString(int error);
v8::Local<v8::Value> TagLibStringToString( TagLib::String s );
TagLib::String NodeStringToTagLibString( v8::Local<v8::Value> s );

struct AsyncBaton {
    uv_work_t request;
    Nan::Persistent<v8::Function> callback;
    int error;

#if _WINDOWS
    char *path; /* only used by read, not save */
#else
    TagLib::FileName path; /* only used by read, not save */
#endif
    // OR
    TagLib::String format;
    BufferStream *stream; // File takes over ownership of the stream
                          // and FileRef takes over ownership of the File
                          // so don't do BufferStream deletion
    Metadata *metadata;
};

void AddResolvers(const Nan::FunctionCallbackInfo< v8::Value >& args);

class CallbackResolver;

struct AsyncResolverBaton {
    uv_async_t request;
    const CallbackResolver *resolver;
#if _WINDOWS
    TagLib::String fileName;
#else
    TagLib::FileName fileName;
#endif
    TagLib::String type;
    uv_async_t idler;
};

class CallbackResolver : public TagLib::FileRef::FileTypeResolver {
    Nan::Persistent<v8::Function> resolverFunc;
#if _WINDOWS
    DWORD created_in;
#else
    const uv_thread_t created_in;
#endif

public:
    CallbackResolver(v8::Local<v8::Function> func);
    TagLib::File *createFile(TagLib::FileName fileName, bool readAudioProperties, TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;
    static void invokeResolverCb(uv_async_t *handle);
    static void stopIdling(uv_async_t *handle);
    static void invokeResolver(AsyncResolverBaton *baton);
};

}
#endif

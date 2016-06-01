#ifndef NODE_TAGLIB_TAG_H
#define NODE_TAGLIB_TAG_H

#include <taglib.h>
#include <fileref.h>
#include <node.h>
#include <uv.h>
#include <nan.h>
#include <nan_object_wrap.h>

namespace node_taglib {
class Metadata : public Nan::ObjectWrap {

private:
    TagLib::Tag * tag;
    TagLib::FileRef *fileRef;

  //static v8::Persistent<v8::FunctionTemplate> pft;

public:
    static NAN_MODULE_INIT(Init);
    Metadata(TagLib::FileRef * fileRef);
    ~Metadata();
    
    static inline Nan::Persistent<v8::Function> & constructor() {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
    }

    /* Tag fields */
    static NAN_GETTER(GetTitle);
    static NAN_SETTER(SetTitle);
    
    static NAN_GETTER(GetArtist);
    static NAN_SETTER(SetArtist);
    
    static NAN_GETTER(GetAlbum);
    static NAN_SETTER(SetAlbum);
    
    static NAN_GETTER(GetYear);
    static NAN_SETTER(SetYear);
    
    static NAN_GETTER(GetComment);
    static NAN_SETTER(SetComment);
    
    static NAN_GETTER(GetTrack);
    static NAN_SETTER(SetTrack);
    
    static NAN_GETTER(GetGenre);
    static NAN_SETTER(SetGenre);

    /* Audio properties */
    static NAN_GETTER(GetAudioLength);
    static NAN_GETTER(GetAudioBitrate);
    static NAN_GETTER(GetAudioSampleRate);
    static NAN_GETTER(GetAudioChannels);
    
    /* Methods */
    static NAN_METHOD(IsEmptyMetadata);
    static NAN_METHOD(CloseMetadata);
    static NAN_METHOD(AsyncSaveMetadata);
    static NAN_METHOD(SyncSaveMetadata);
    static NAN_METHOD(SyncReadMetadata);
    static NAN_METHOD(AsyncReadMetadata);
    
    static void AsyncMetadataReadDo(uv_work_t *req);
    static void AsyncMetadataReadAfter(uv_work_t *req);
    static void AsyncSaveMetadataDo(uv_work_t *req);
    static void AsyncSaveMetadataAfter(uv_work_t *req);
};
}
#endif

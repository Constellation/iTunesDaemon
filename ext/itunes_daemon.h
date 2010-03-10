#ifndef _ITUNES_DAEMON_H_
#define _ITUNES_DAEMON_H_
#include <vector>
#include <iTunesCOMInterface.h>
#include "iTunesCOMInterface_i.c"

extern NPNetscapeFuncs *npnfuncs;

class NPAPI {
 public:
  static const char * const kMethodPost;
  static const char * const kMethodCurrentTrack;
  static NPMIMEType kMIMETypeDescription;
  static struct NPClass kNpcRefObject;
  static struct NPClass kStringHashObject;

  explicit NPAPI(NPP*);
  ~NPAPI();

  inline static bool BSTRToUTF8(const BSTR* bstr, std::string& str){
    std::size_t len = WideCharToMultiByte(CP_UTF8, 0, *bstr, -1, NULL, 0, NULL, NULL);
    LPSTR s = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, *bstr, -1, s, len, NULL, NULL);
    str = std::string(s);
    delete [] s;
    return true;
  }

  inline NPObject* NPObjectValue() const {
    return npobject_;
  }

  inline NPP Instance() const {
    return *instance_;
  }

  static bool StringToNPVariant(const std::string&, NPVariant*);
  static bool HasMethod(NPObject*, NPIdentifier);
  static bool Invoke(NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant*);
  static NPNetscapeFuncs * Npnfuncs;
 private:
  NPP *instance_;
  NPObject * npobject_;
};

const char * const NPAPI::kMethodPost = "post";
const char * const NPAPI::kMethodCurrentTrack = "currentTrack";
NPMIMEType NPAPI::kMIMETypeDescription = const_cast<NPMIMEType>("application/x-chrome-npapi-itunes-daemon:.:itunes-daemon@utatane.tea");
struct NPClass NPAPI::kNpcRefObject = {
  NP_CLASS_STRUCT_VERSION,
  NULL,
  NULL,
  NULL,
  NPAPI::HasMethod,
  NPAPI::Invoke,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

class Track : public NPObject {
 public:
  static const char * const kPropertyName;
  static const char * const kPropertyArtist;
  static const char * const kPropertyAlbum;
  static const char * const kPropertyGenre;
  static const uint32_t kEnumCount = 4;
  static NPClass kNPClass;

  explicit Track(void):available_(false){}
  inline const std::string& Name(void) const{
    return name_;
  }
  inline const std::string& Artist(void) const{
    return artist_;
  }
  inline const std::string& Album(void) const{
    return album_;
  }
  inline const std::string& Genre(void) const{
    return genre_;
  }
  inline const bool Available(void) const{
    return available_;
  }
  const bool Init(IITTrack*);
  void Retain(void);

  static Track* New(NPP npp);
  static NPObject* Allocate(NPP npp, NPClass *klass){
    return static_cast<NPObject *>(new Track());
  }
  static Track* Allocate(){
    return new Track();
  }

  static void _Deallocate(NPObject*);
  static void _Invalidate(NPObject*);
  static bool _HasMethod(NPObject*, NPIdentifier);
  static bool _Invoke(NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant*);
  static bool _InvokeDefault(NPObject*, const NPVariant*, uint32_t, NPVariant*);
  static bool _HasProperty(NPObject*, NPIdentifier);
  static bool _GetProperty(NPObject*, NPIdentifier, NPVariant*);
  static bool _SetProperty(NPObject*, NPIdentifier, const NPVariant*);
  static bool _RemoveProperty(NPObject*, NPIdentifier);
  static bool _Enumeration(NPObject*, NPIdentifier**, uint32_t*);
  static bool _Construct(NPObject*, const NPVariant*, uint32_t, NPVariant*);

 protected:
  void Deallocate(void);
  void Invalidate(void);
  bool HasMethod(NPIdentifier);
  bool Invoke(NPIdentifier, const NPVariant*, uint32_t, NPVariant*);
  bool InvokeDefault(const NPVariant*, uint32_t, NPVariant*);
  bool HasProperty(NPIdentifier);
  bool GetProperty(NPIdentifier, NPVariant*);
  bool SetProperty(NPIdentifier, const NPVariant*);
  bool RemoveProperty(NPIdentifier);
  bool Enumeration(NPIdentifier**, uint32_t*);
  bool Construct(const NPVariant*, uint32_t, NPVariant*);

 private:
  std::string name_;
  std::string artist_;
  std::string album_;
  std::string genre_;
  bool available_;
};

struct NPClass Track::kNPClass = {
  NP_CLASS_STRUCT_VERSION,
  Track::Allocate,
  Track::_Deallocate,
  NULL,//Track::_Invalidate,
  NULL,//Track::_HasMethod,
  NULL,//Track::_Invoke,
  NULL,//Track::_InvokeDefault,
  Track::_HasProperty,
  Track::_GetProperty,
  NULL,//Track::_SetProperty,
  NULL,//Track::_RemoveProperty,
  NULL,//Track::_Enumeration,
  NULL,//Track::_Construct,
};

const char * const Track::kPropertyName   = "name";
const char * const Track::kPropertyArtist = "artist";
const char * const Track::kPropertyAlbum  = "album";
const char * const Track::kPropertyGenre  = "genre";

class iTunes {
 public:
  iTunes():
    available_(false)
  {
    HRESULT hRes;
    hRes = ::CoCreateInstance(CLSID_iTunesApp, NULL,
      CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iTunes_);
    available_ = hRes == S_OK;
  }
  ~iTunes(){
    if(available_){
      iTunes_->Release();
    }
  }
  bool Playing(void) const {
    return false;
  }
  Track* CurrentTrack(void) const;
  inline const bool Available(void) const {
    return available_;
  }
 protected:
 private:
  IiTunes *iTunes_;
  bool available_;
};

#endif//_ITUNES_DAEMON_H_

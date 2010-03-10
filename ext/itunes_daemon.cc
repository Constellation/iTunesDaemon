#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <windows.h>

#include <npapi/npapi.h>
#include <npapi/nptypes.h>
#include <npapi/npruntime.h>
#include <npapi/npfunctions.h>

#include <string>
#include <memory>


#ifdef DEBUG
static inline void
logmsg(const char *msg)
{
#ifdef WIN32
  std::FILE *out;
  fopen_s(&out, "c:\\itunes.log", "abN");
#else
  std::FILE *out = std::fopen("/tmp/itunes.log", "ab");
#endif
  std::fprintf(out, "%s\n", msg);
  std::fclose(out);
}
#endif

#include "itunes_daemon.h"

static NPAPI *npapi;
static NPNetscapeFuncs *npnfuncs;

const bool
Track::Init(IITTrack* track)
{
  available_ = true;
  if(available_){
    BSTR bstr_name;
    track->get_Name(&bstr_name);
    NPAPI::BSTRToUTF8(&bstr_name, name_);
    ::SysFreeString(bstr_name);

    BSTR bstr_artist;
    track->get_Artist(&bstr_artist);
    NPAPI::BSTRToUTF8(&bstr_artist, artist_);
    ::SysFreeString(bstr_artist);

    BSTR bstr_album;
    track->get_Album(&bstr_album);
    NPAPI::BSTRToUTF8(&bstr_album, album_);
    ::SysFreeString(bstr_album);

    BSTR bstr_genre;
    track->get_Genre(&bstr_genre);
    NPAPI::BSTRToUTF8(&bstr_genre, genre_);
    ::SysFreeString(bstr_genre);
  }
  return true;
}

void
Track::Retain()
{
  npnfuncs->retainobject(static_cast<NPObject*>(this));
}

Track*
Track::New(NPP npp)
{
  return static_cast<Track*>(npnfuncs->createobject(npp, &kNPClass));
}

void
Track::Deallocate()
{
}
void
Track::_Deallocate(NPObject *obj)
{
  delete static_cast<Track*>(obj);
}

void
Track::Invalidate()
{
}
void
Track::_Invalidate(NPObject *obj)
{
  static_cast<Track*>(obj)->Invalidate();
}

bool
Track::HasMethod(NPIdentifier name){
  return false;
}
bool
Track::_HasMethod(NPObject *obj, NPIdentifier name)
{
  return static_cast<Track*>(obj)->HasMethod(name);
}

bool
Track::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  return false;
}
bool
Track::_Invoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  return static_cast<Track*>(obj)->Invoke(name, args, argCount, result);
}

bool
Track::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  return false;
}
bool
Track::_InvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  return static_cast<Track*>(obj)->InvokeDefault(args, argCount, result);
}

bool
Track::HasProperty(NPIdentifier ident)
{
  char *name = npnfuncs->utf8fromidentifier(ident);
  bool result = false;
  result = (std::strcmp(name, kPropertyName) == 0)   ||
           (std::strcmp(name, kPropertyArtist) == 0) ||
           (std::strcmp(name, kPropertyAlbum) == 0)  ||
           (std::strcmp(name, kPropertyGenre) == 0);
  npnfuncs->memfree(name);
  return result;
}
bool
Track::_HasProperty(NPObject *obj, NPIdentifier name)
{
  return static_cast<Track*>(obj)->HasProperty(name);
}

bool
Track::GetProperty(NPIdentifier ident, NPVariant *result)
{
  char *name = npnfuncs->utf8fromidentifier(ident);
  bool res = false;
  if(std::strcmp(name, kPropertyName) == 0){
    NPAPI::StringToNPVariant(name_, result);
    res = true;
  } else if(std::strcmp(name, kPropertyArtist) == 0){
    NPAPI::StringToNPVariant(artist_, result);
    res = true;
  } else if(std::strcmp(name, kPropertyAlbum) == 0){
    NPAPI::StringToNPVariant(album_, result);
    res = true;
  } else if(std::strcmp(name, kPropertyGenre) == 0){
    NPAPI::StringToNPVariant(genre_, result);
    res = true;
  }
  npnfuncs->memfree(name);
  return res;
}
bool
Track::_GetProperty(NPObject *obj, NPIdentifier name, NPVariant *result)
{
  return static_cast<Track*>(obj)->GetProperty(name, result);
}

// read only
bool
Track::SetProperty(NPIdentifier name, const NPVariant *value)
{
  return false;
}
bool
Track::_SetProperty(NPObject *obj, NPIdentifier name, const NPVariant *value)
{
  return static_cast<Track*>(obj)->SetProperty(name, value);
}

// unremovable
bool
Track::RemoveProperty(NPIdentifier name)
{
  return false;
}
bool
Track::_RemoveProperty(NPObject *obj, NPIdentifier name)
{
  return static_cast<Track*>(obj)->RemoveProperty(name);
}

bool
Track::Enumeration(NPIdentifier **identifiers, uint32_t *count)
{
  // webkit bug
  return false;
}
bool
Track::_Enumeration(NPObject *obj, NPIdentifier **identifiers, uint32_t *count)
{
  return static_cast<Track*>(obj)->Enumeration(identifiers, count);
}

bool
Track::Construct(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  return false;
}
bool
Track::_Construct(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  return static_cast<Track*>(obj)->Construct(args, argCount, result);
}

Track*
iTunes::CurrentTrack(void) const
{
  Track *track = Track::New(npapi->Instance());
  if(available_){
    HRESULT hRes;
    IITTrack *itrack;
    hRes = iTunes_->get_CurrentTrack(&itrack);
    if(hRes == S_OK){
      track->Init(itrack);
      itrack->Release();
      return track;
    } else {
      return track;
    }
  } else {
    return track;
  }
}

bool
NPAPI::StringToNPVariant(const std::string &str, NPVariant *variant)
{
  std::size_t len = str.size();
  NPUTF8 *chars = static_cast<NPUTF8 *>(npnfuncs->memalloc(len));
  if(!chars){
    VOID_TO_NPVARIANT(*variant);
    return false;
  }
  memcpy(chars, str.c_str(), len);
  STRINGN_TO_NPVARIANT(chars, len, *variant);
  return true;
}

NPAPI::NPAPI(NPP* instance):
  instance_(instance)
{
  npobject_ = npnfuncs->createobject(*instance_, &NPAPI::kNpcRefObject);
  npnfuncs->retainobject(npobject_);
}

NPAPI::~NPAPI()
{
  npnfuncs->releaseobject(npobject_);
}

bool
NPAPI::HasMethod(NPObject *obj, NPIdentifier methodName)
{
  char *name = npnfuncs->utf8fromidentifier(methodName);
  bool result = std::strcmp(name, NPAPI::kMethodPost) == 0 || std::strcmp(name, NPAPI::kMethodCurrentTrack) == 0;
  npnfuncs->memfree(name);
  return result;
}

bool
NPAPI::Invoke(NPObject *obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result){
  NPUTF8 *name = npnfuncs->utf8fromidentifier(methodName);
  BOOLEAN_TO_NPVARIANT(false, *result);
  if(std::strcmp(name, NPAPI::kMethodPost) == 0){
    npnfuncs->memfree(name);
    if(argCount < 1){
      npnfuncs->setexception(obj, "Parameter 1 is required.");
      return false;
    }
    BOOLEAN_TO_NPVARIANT(true, *result);
    return true;
  } else if(std::strcmp(name, NPAPI::kMethodCurrentTrack) == 0){
    npnfuncs->memfree(name);
    if(argCount > 0){
      npnfuncs->setexception(obj, "Parameter isn't required.");
      return false;
    }
    iTunes itunes;
    if(itunes.Available()){
      Track *track = itunes.CurrentTrack();
      if(track->Available()){
        NPObject *object = static_cast<NPObject*>(track);
        OBJECT_TO_NPVARIANT(object, *result);
        return true;
      } else {
        delete track;
        NULL_TO_NPVARIANT(*result);
        return true;
      }
    } else {
      NULL_TO_NPVARIANT(*result);
      return true;
    }
  } else {
    npnfuncs->memfree(name);
    npnfuncs->setexception(obj, "no such method.");
    return false;
  }
}

#ifdef __cplusplus
extern "C" {
#endif


static NPError
nevv(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char *argn[], char *argv[], NPSavedData *saved)
{
  return NPERR_NO_ERROR;
}

static NPError
destroy(NPP instance, NPSavedData **save)
{
  delete npapi;
  npapi = NULL;
  ::CoUninitialize();
  return NPERR_NO_ERROR;
}

// getValue function when getter called
static NPError
getValue(NPP instance, NPPVariable variable, void *value)
{
  switch(variable){
  case NPPVpluginNameString:
    *(static_cast<const char **>(value)) = "iTunes daemon";
    break;
  case NPPVpluginDescriptionString:
    *(static_cast<const char **>(value)) = "NPAPI extension to post music to iTunes";
    break;
  case NPPVpluginScriptableNPObject:
    if(!npapi){
      ::CoInitialize(NULL);
      npapi = new NPAPI(&instance);
    }
    *(NPObject **)value = npapi->NPObjectValue();
    break;
#ifdef XUL_RUNNER_SDK
  case NPPVpluginNeedsXEmbed:
    break;
#endif
  default:
    return NPERR_GENERIC_ERROR;
  }
  return NPERR_NO_ERROR;
}

NPError OSCALL
NP_GetEntryPoints(NPPluginFuncs *nppfuncs)
{
  nppfuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  nppfuncs->size    = sizeof(*nppfuncs);
  nppfuncs->newp    = nevv;
  nppfuncs->destroy = destroy;
  nppfuncs->getvalue= getValue;
  return NPERR_NO_ERROR;
}

NPError OSCALL
NP_Initialize(NPNetscapeFuncs *npnf
#if !defined(WIN32) && !defined(WEBKIT_DARWIN_SDK)
    ,NPPluginFuncs *nppfuncs
#endif
)
{
  if(npnf == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;

  if((npnf->version >> 8) > NP_VERSION_MAJOR)
    return NPERR_INCOMPATIBLE_VERSION_ERROR;

  npnfuncs = npnf;
#if !defined(WIN32) && !defined(WEBKIT_DARWIN_SDK)
  NP_GetEntryPoints(nppfuncs);
#endif
  return NPERR_NO_ERROR;
}

NPError OSCALL
NP_Shutdown(void)
{
  return NPERR_NO_ERROR;
}

char*
NP_GetMIMEDescription(void)
{
  return NPAPI::kMIMETypeDescription;
}

NPError OSCALL
NP_GetValue(void *npp, NPPVariable variable, void *value)
{
  return getValue((NPP)npp, variable, value);
}

#ifdef __cplusplus
}
#endif



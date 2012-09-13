/**
 * @file AudioDeviceLibrary.c
 * @brief Helper for C clients of the AudioDevice library.
 * Copyright 2007 The MathWorks, Inc.
 * $Revision: 1.1.10.2 $ $Date: 2007/12/10 21:31:25 $
 */

#include "AudioDevice_rtw.h"
#include <string.h>
#include <stdio.h>
#if defined(_WIN32)

static const char *libName = "libaudiodevice.dll";

#elif defined(__APPLE__)

static const char *libName = "libaudiodevice.dylib";

#else

static const char *libName = "libaudiodevice.so";

#endif

/* Close the library */
void DestroyAudioDeviceLibrary(AudioDeviceLibrary *adl)
{
  if (adl->library) {

#if defined(DEBUG)

    /* we link against the library directly in this caes */
#elif defined(_WIN32)

    FreeLibrary(adl->library);

#else

    dlclose(adl->library);

#endif

    memset(&adl, 0, sizeof(adl));
  }
}

/* Open the library and initialize the function pointers */
AudioDeviceLibrary CreateAudioDeviceLibrary(char *err)
{
  AudioDeviceLibrary adl;
  memset(&adl, 0, sizeof(adl));

#if defined(DEBUG)

  adl.deviceCreate = deviceCreate;
  adl.deviceStart = deviceStart;
  adl.deviceUpdate = deviceUpdate;
  adl.deviceOutputs = deviceOutputs;
  adl.deviceTerminate = deviceTerminate;
  adl.deviceDestroy = deviceDestroy;

#elif defined(_WIN32)

  adl.library = LoadLibrary(libName);
  if (adl.library == NULL)
    sprintf(err, "Could not open library: %s", libName);
  else {
    adl.deviceCreate = (pFnDeviceCreate) GetProcAddress(adl.library,
      "deviceCreate");
    adl.deviceStart = (pFnDeviceStart) GetProcAddress(adl.library, "deviceStart");
    adl.deviceUpdate = (pFnDeviceUpdate) GetProcAddress(adl.library,
      "deviceUpdate");
    adl.deviceOutputs = (pFnDeviceOutputs) GetProcAddress(adl.library,
      "deviceOutputs");
    adl.deviceTerminate = (pFnDeviceTerminate) GetProcAddress(adl.library,
      "deviceTerminate");
    adl.deviceDestroy = (pFnDeviceDestroy) GetProcAddress(adl.library,
      "deviceDestroy");
  }

#else

  adl.library = dlopen(libName, RTLD_NOW | RTLD_LOCAL);
  if (adl.library == NULL) {
    sprintf(err, "Could not open library: %s", libName);
  } else {
    /* The rather nasty casting below is a result of the following gcc warning message:
       warning: ISO C forbids conversion of object pointer to function pointer type
     */
    *(void **) (&adl.deviceCreate) = dlsym(adl.library, "deviceCreate");
    *(void **) (&adl.deviceStart) = dlsym(adl.library, "deviceStart");
    *(void **) (&adl.deviceUpdate) = dlsym(adl.library, "deviceUpdate");
    *(void **) (&adl.deviceOutputs) = dlsym(adl.library, "deviceOutputs");
    *(void **) (&adl.deviceTerminate) = dlsym(adl.library, "deviceTerminate");
    *(void **) (&adl.deviceDestroy) = dlsym(adl.library, "deviceDestroy");
  }

#endif

  if (!*err &&
      (!adl.deviceCreate ||
       !adl.deviceStart ||
       !adl.deviceUpdate ||
       !adl.deviceOutputs ||
       !adl.deviceTerminate ||
       !adl.deviceDestroy)
      ) {
    sprintf(err, "Could not determine function entry points in %s", libName);
  }

  if (*err && adl.library)
    DestroyAudioDeviceLibrary(&adl);
  return adl;
}

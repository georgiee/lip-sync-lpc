/**
 * @file AudioDevice_rtw.h
 * @brief Helper for C clients of the AudioDevice library.
 * Copyright 2007-2008 The MathWorks, Inc.
 * $Revision: 1.1.10.3.4.1 $ $Date: 2008/01/30 20:17:41 $
 */

/* include headers that we need for opening dynamic libraries */
#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

/* define DllExport */
#if defined(_WIN32) && !defined(__LCC__)
#  if !defined(DllExport)
#    define DllExport                  __declspec(dllexport)
#  endif

#else
#  define DllExport
#endif

/* Wrap everything in extern C */
#ifdef __cplusplus

extern "C" {

#endif

  typedef DllExport void (*pFnDeviceCreate)(char *err, char *warn, const char
    *deviceName, int inOut, void **m_device,
    int numChannels, double sampleRate, int deviceDatatype, int bufferSize,
    double queueDuration,
    int frameSize, int signalDatatype);
  DllExport void deviceCreate(char *err, char *warn, const char *deviceName, int
    inOut, void **m_device,
    int numChannels, double sampleRate, int deviceDatatype, int bufferSize,
    double queueDuration,
    int frameSize, int signalDatatype);
  typedef DllExport void (*pFnDeviceStart)(void *device, char *err);
  DllExport void deviceStart(void *device, char *err);
  typedef DllExport void (*pFnDeviceUpdate)(void *device, char *err, const void *
    src, int signalDatatype, int samplesPerFrame);
  DllExport void deviceUpdate(void *device, char *err, const void *src, int
    signalDatatype, int samplesPerFrame);
  typedef DllExport void (*pFnDeviceOutputs)(void *device, char *err, void *src,
    int signalDatatype, int samplesPerFrame);
  DllExport void deviceOutputs(void *device, char *err, void *src, int
    signalDatatype, int samplesPerFrame);
  typedef DllExport void (*pFnDeviceTerminate)(void *device, char *err);
  DllExport void deviceTerminate(void *device, char *err);
  typedef DllExport void (*pFnDeviceDestroy)(void *device, char *err, int type);
  DllExport void deviceDestroy(void *device, char *err, int type);

  /* Define a table of function pointers and the library handle */
  typedef struct {

#ifdef _WIN32

    HMODULE library;

#else

    void *library;

#endif

    pFnDeviceCreate deviceCreate;
    pFnDeviceStart deviceStart;
    pFnDeviceUpdate deviceUpdate;
    pFnDeviceOutputs deviceOutputs;
    pFnDeviceTerminate deviceTerminate;
    pFnDeviceDestroy deviceDestroy;
  } AudioDeviceLibrary;

  /* Close the library */
  void DestroyAudioDeviceLibrary(AudioDeviceLibrary *adl);

  /* Open the library and initialize the function pointers */
  AudioDeviceLibrary CreateAudioDeviceLibrary(char *err);

#ifdef __cplusplus

}                                      // extern "C"
#endif

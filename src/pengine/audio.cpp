
// audio.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"


// Available:
//
//  USE_NULL
//  USE_OPENAL
//  USE_FMOD
//  USE_SDL_MIXER
//
// ... and they're all squashed into this one file!


#ifdef UNIX
//#define USE_NULL
#define USE_OPENAL
#endif

#ifdef WIN32
//#define USE_NULL
//#define USE_OPENAL
#define USE_FMOD
#endif


// I was half way through implementing SDL_mixer support when
// I realised it has no ability to change pitch / playback
// speed. Don't enable USE_SDL_MIXER.

//#define USE_SDL_MIXER



// This function is common to the various implementations

PAudioSample *PSSAudio::loadSample(const std::string &name, bool positional3D)
{
  PAudioSample *samp = samplist.find(name);
  
  if (!samp)
  {
    try
    {
      samp = new PAudioSample(name, positional3D);
    }
    catch (PException e)
    {
      if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
        PUtil::outLog() << "Failed to load " << name << ": " << e.what () << std::endl;
      return null;
    }
    samplist.add(samp);
  }
  return samp;
}





#ifdef USE_NULL

PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
  PUtil::outLog() << "Initialising audio subsystem [NULL]" << std::endl;
}

PSSAudio::~PSSAudio()
{
  PUtil::outLog() << "Shutting down audio subsystem" << std::endl;
  
  samplist.clear();
}


PAudioSample *PSSAudio::loadSample(const std::string &name, bool positional3D)
{
  PAudioSample *samp = samplist.find(name);
  
  return samp;
}

PAudioSample::PAudioSample (const std::string &filename, bool positional3D)
{
  buffer = 0;
  
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;
  
  unload();
  
  name = filename;
}

void PAudioSample::unload()
{
}

PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping) :
  samp(_samp)
{
}

PAudioInstance::~PAudioInstance()
{
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
}

void PAudioInstance::setGain(float gain)
{
}

void PAudioInstance::setHalfDistance(float lambda)
{
}

void PAudioInstance::setPitch(float pitch)
{
}


void PAudioInstance::play()
{
}

void PAudioInstance::stop()
{
}

bool PAudioInstance::isPlaying()
{
  return false;
}

#endif // USE_NULL


#ifdef USE_OPENAL


#include <AL/al.h>
//#include <AL/alu.h> // not available in newest OpenAL
#include <AL/alut.h>


PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
  PUtil::outLog() << "Initialising audio subsystem [OpenAL]" << std::endl;
  
  if (alutInit(0, null) != AL_TRUE) {
    throw MakePException("ALUT:alutInit() error: " 
                                  +  alutGetErrorString(alutGetError()));
  }
}

PSSAudio::~PSSAudio()
{
  PUtil::outLog() << "Shutting down audio subsystem" << std::endl;

  samplist.clear();

  alutExit();
}


PAudioSample::PAudioSample (const std::string &filename, bool positional3D)
{
  buffer = 0;
  positional3D = positional3D; // unused (atm)
  
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;
  
  unload();
  /* load contents from file into memory using physfs functions */
  name = filename;
  PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());
  
  if (!pfile) {
    throw MakePException ("Load failed: PhysFS: " + PHYSFS_getLastError());
  }
  
  int filesize = PHYSFS_fileLength(pfile);
  
  char *wavbuffer = new char[filesize];
  
  PHYSFS_read(pfile, wavbuffer, filesize, 1);
  PHYSFS_close(pfile);

  /* create the alut buffer from memory contents */
  this->buffer = alutCreateBufferFromFileImage(
         reinterpret_cast<const ALvoid *>(wavbuffer),
         filesize);

  /* clean up */
  delete [] wavbuffer;

  /* check if loading was successful */
  if (AL_NONE == this->buffer) {
    throw MakePException("Sample load failed:" 
                 + alutGetErrorString(alutGetError()));
  }
}

void PAudioSample::unload()
{
  if (buffer) {
    alDeleteBuffers(1, &buffer);
    buffer = 0;
  }
}



PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping)
{
  samp = _samp;

  alGenSources(1, &source);

  alSourcei(source, AL_BUFFER, samp->buffer);
  alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);

  alSourcePlay(source);
}

PAudioInstance::~PAudioInstance()
{
  if (isPlaying()) stop();
  alDeleteSources(1, &source);
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
  alSourcefv(source, AL_POSITION, (vec3f)pos);
  alSourcefv(source, AL_VELOCITY, (vec3f)vel);
}

void PAudioInstance::setGain(float gain)
{
  //alSourcef(source, AL_MIN_GAIN, gain);
  //alSourcef(source, AL_MAX_GAIN, gain);
  alSourcef(source, AL_GAIN, gain);
}

void PAudioInstance::setHalfDistance(float lambda)
{
  alSourcef(source, AL_REFERENCE_DISTANCE, lambda);
}

void PAudioInstance::setPitch(float pitch)
{
  alSourcef(source, AL_PITCH, pitch);
}


void PAudioInstance::play()
{
  alSourceRewind(source);
  alSourcePlay(source);
}

void PAudioInstance::stop()
{
  alSourceStop(source);
}

bool PAudioInstance::isPlaying()
{
  int state = AL_STOPPED;
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

#endif // USE_OPENAL


#ifdef USE_FMOD


//#error Need to integrate PhysFS with FMOD

#include <fmod.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


void * F_CALLBACKAPI fsound_open(const char *name)
{
  PHYSFS_file *pfile = PHYSFS_openRead(name);
  
  if (!pfile)
    PUtil::outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
  
  return pfile;
}

void F_CALLBACKAPI fsound_close(void *handle)
{
  PHYSFS_file *pfile = reinterpret_cast< PHYSFS_file * > (handle);

  if (pfile)
    PHYSFS_close(pfile);
}

int F_CALLBACKAPI fsound_read(void *buffer, int size, void *handle)
{
  PHYSFS_file *pfile = reinterpret_cast< PHYSFS_file * > (handle);
  
  return PHYSFS_read(pfile, buffer, size, 1);
}

int F_CALLBACKAPI fsound_seek(void *handle, int pos, signed char mode)
{
  PHYSFS_file *pfile = reinterpret_cast< PHYSFS_file * > (handle);
  
  int target;
  
  int curpos = PHYSFS_tell(pfile);
  
  switch (mode) {
  default:
  case SEEK_SET:
    target = pos;
    break;
  case SEEK_CUR:
    target = curpos + pos;
    break;
  case SEEK_END:
    target = PHYSFS_fileLength(pfile) + pos;
    break;
  }
  
  PHYSFS_seek(pfile, target);
  
  return curpos;
}

int F_CALLBACKAPI fsound_tell(void *handle)
{
  PHYSFS_file *pfile = reinterpret_cast< PHYSFS_file * > (handle);
  
  return PHYSFS_tell(pfile);
}


PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
  PUtil::outLog() << "Initialising audio subsystem [FMOD]" << std::endl;
  
  if (!FSOUND_Init(22050, 16, 0)) {
    throw MakePException ("FMOD initialisation failed");
  }
  
  FSOUND_File_SetCallbacks(
    fsound_open,
    fsound_close,
    fsound_read,
    fsound_seek,
    fsound_tell);
}

PSSAudio::~PSSAudio()
{
  PUtil::outLog() << "Shutting down audio subsystem" << std::endl;

  samplist.clear();

  FSOUND_Close();
}



PAudioSample::PAudioSample (const std::string &filename, bool positional3D)
{
  buffer = 0;
  
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;

  unload();

  name = filename;

  buffer = (uint32) FSOUND_Sample_Load(FSOUND_FREE, filename.c_str(),
    positional3D ? FSOUND_HW3D : FSOUND_HW2D, 0, 0);

  if (!buffer) {
    throw MakePException ("Sample load failed");
  }
}

void PAudioSample::unload()
{
  if (buffer) {
    FSOUND_Sample_Free((FSOUND_SAMPLE *)buffer);
    buffer = 0;
  }
}



PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping) :
  samp(_samp)
{
  //FSOUND_Sample_SetMode((FSOUND_SAMPLE *)samp->buffer,
  //    looping ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);

  source = (uint32) FSOUND_PlaySoundEx(FSOUND_FREE,
    (FSOUND_SAMPLE *)samp->buffer, null, TRUE);

  *((float*)&reserved1) = (float)FSOUND_GetFrequency((int)source);

  FSOUND_SetLoopMode((int)source,
    looping ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);
}

PAudioInstance::~PAudioInstance()
{
  if (isPlaying()) stop();
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
  // TODO
}

void PAudioInstance::setGain(float gain)
{
  CLAMP(gain, 0.0f, 1.0f);
  FSOUND_SetVolume((int)source, (int)(gain * 255.0f));
}

void PAudioInstance::setHalfDistance(float lambda)
{
  // TODO
}

void PAudioInstance::setPitch(float pitch)
{
  FSOUND_SetFrequency((int)source, (int)(pitch * *((float*)&reserved1)));
}


void PAudioInstance::play()
{
  FSOUND_SetPaused((int)source, FALSE);
}

void PAudioInstance::stop()
{
  FSOUND_StopSound((int)source);
}

bool PAudioInstance::isPlaying()
{
  return (FSOUND_IsPlaying((int)source) == TRUE);
}

#endif // USE_FMOD


#ifdef USE_SDL_MIXER

#include <SDL_mixer.h>

PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
  PUtil::outLog() << "Initialising audio subsystem [SDL_mixer]" << std::endl;
  
  if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 1, 2048) != 0) {
    PUtil::outLog() << "SDL_mixer failed to initialise" << std::endl;
    PUtil::outLog() << "SDL_mixer: " << Mix_GetError() << std::endl;
    return PException ();
  }
}

PSSAudio::~PSSAudio()
{
  PUtil::outLog() << "Shutting down audio subsystem" << std::endl;
  
  samplist.clear();
  
  Mix_CloseAudio();
}


PAudioSample::PAudioSample (const std::string &filename, bool positional3D)
{
  buffer = 0;
  
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;
  
  unload();
  
  name = filename;
  
  PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());
  
  if (!pfile) {
    PUtil::outLog() << "Load failed: PhysFS: " << PHYSFS_getLastError() << std::endl;
    throw PFileException ();
  }
  
  buffer = (uint32) Mix_LoadWAV_RW(PUtil::allocPhysFSops(pfile), 1);
  
  PHYSFS_close(pfile);
  
  if (!buffer) {
    PUtil::outLog() << "Sample load failed" << std::endl;
    PUtil::outLog() << "SDL_mixer: " << Mix_GetError() << std::endl;
    throw PFileException ();
  }
}

void PAudioSample::unload()
{
  if (buffer)
    Mix_FreeChunk((Mix_Chunk *) buffer);
    buffer = 0;
  }
}


PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping) :
  samp(_samp)
{
  source = (uint32) Mix_PlayChannel(-1
    (Mix_Chunk *)samp->buffer, looping ? -1 : 0);

  *((float*)&reserved1) = (float)FSOUND_GetFrequency((int)source);

  FSOUND_SetLoopMode((int)source,
    looping ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);
}

PAudioInstance::~PAudioInstance()
{
  if (source != -1) {
  }
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
  // TODO
}

void PAudioInstance::setGain(float gain)
{
}

void PAudioInstance::setHalfDistance(float lambda)
{
  // TODO
}

void PAudioInstance::setPitch(float pitch)
{
}


void PAudioInstance::play()
{
}

void PAudioInstance::stop()
{
}

bool PAudioInstance::isPlaying()
{
  return false;
}

#endif // USE_SDL_MIXER






// audio.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


// Ubuntu patch "15_fixup_physfs_alut_interaction.patch" includes
// the OpenAL header so that ALuint can be used for PAudioSample::buffer.
// I don't like exposing OpenAL to the rest of the engine, so I'm
// omitting it, but if you find it causes build problems, you may
// want to re-enable it below. - jaz 20/10/2006

#if 0 // defined( USE_OPENAL )
#define INCLUDE_OPENAL_HEADER
#endif

#if defined( INCLUDE_OPENAL_HEADER )
#include <AL/al.h>
#endif

class PSSAudio : public PSubsystem {
private:
    PResourceList<PAudioSample> samplist;

public:
    PSSAudio(PApp &parentApp);
    ~PSSAudio();

    // TODO: tick

    PAudioSample *loadSample(const std::string &name, bool positional3D = true);
};

class PAudioSample : public PResource {
private:
#if defined( INCLUDE_OPENAL_HEADER )
    ALuint buffer;
#else
    unsigned int buffer;
#endif

public:
    PAudioSample(const std::string &filename, bool positional3D = false);
    ~PAudioSample() { unload(); }

    void unload();

    friend class PAudioInstance;
};

class PAudioInstance {
private:
    PAudioSample *samp;
    //uint32 source;
    //uint32 reserved1;
    unsigned int source;
    unsigned int reserved1;

public:
    PAudioInstance(PAudioSample *_samp, bool looping = false);
    ~PAudioInstance();

    void update(const vec3f &pos, const vec3f &vel);
    void setGain(float gain); // 0-1
    void setHalfDistance(float lambda);
    void setPitch(float pitch); // 1 is normal

    void play();
    void stop();
    bool isPlaying();
};


#ifdef INCLUDE_OPENAL_HEADER
#undef INCLUDE_OPENAL_HEADER
#endif


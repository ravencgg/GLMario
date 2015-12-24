#include <windows.h>
#include <dsound.h>

#include "audio.h"
#include "types.h"
#include "game_types.h"
#include "utility.h"

#include "SDL_audio.h"

static const uint32 CC_riff = 'RIFF';
static const uint32 CC_fmt  = 'fmt ';
static const uint32 CC_data = 'data';

namespace
{
    // TODO: HANDLE audio_thread;
    //
    uint8* audio_chunk;
    int32 audio_len;
    uint8* audio_pos;
}

void FillAudio(void* data, uint8* stream, int32 len)
{
    static int done = 0;

    memset(stream, 0, len);

    if(audio_len == 0)
    {
        return;
    }

#if 0
    if (done > 3) return;
    printf("running");
    ++done;
#endif

    len = ( len > audio_len ? audio_len : len );

    SDL_MixAudioFormat(stream, audio_pos, AUDIO_S16, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

bool InitializeAudio(SDL_AudioSpec* audio_spec)
{

#if 0
    SDL_AudioSpec audio_spec;
    audio_spec.freq = 22050;
    audio_spec.format = AUDIO_S16;
    audio_spec.channels = 2;
    audio_spec.samples = 1024;
    audio_spec.callback = FillAudio;
    audio_spec.userdata = NULL;
#endif

    if(SDL_OpenAudio(audio_spec, NULL) < 0)
    {
        printf("Unable to start audio\n");
        return false;
    }

    return true;
}

void ShutdownAudio()
{
    SDL_CloseAudio();
}

void PauseAudio(bool pause)
{
    SDL_PauseAudio(pause);
}

bool LoadWavFile(const char* filename)
{
    SDL_AudioSpec audio_spec;
    uint32 wav_length;
    uint8* wav_buffer;
#if 0
    audio_spec.freq = 22050;
    audio_spec.format = AUDIO_S16;
    audio_spec.channels = 2;
    audio_spec.samples = 1024;
#endif

    if(SDL_LoadWAV(filename, &audio_spec, &wav_buffer, &wav_length))
    {
        audio_chunk = wav_buffer;
        audio_pos = audio_chunk;
        audio_len = wav_length;

        audio_spec.userdata = NULL;
        audio_spec.callback = FillAudio;


        InitializeAudio(&audio_spec);

        return true;
    }
    return false;
}




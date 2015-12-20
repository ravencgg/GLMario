#include <windows.h>
#include <dsound.h>

#include "audio.h"
#include "types.h"
#include "game_types.h"
#include "utility.h"

#include "SDL_syswm.h"

static const uint32 CC_riff = 'RIFF';
static const uint32 CC_fmt  = 'fmt ';
static const uint32 CC_data = 'data';

typedef HRESULT (WINAPI *DirectSoundCreateFunc)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

namespace
{
    HMODULE audio_dll;
    LPDIRECTSOUND direct_sound;
    // TODO: HANDLE audio_thread;
}

bool InitializeAudio(Window* window)
{
    audio_dll = LoadLibraryA("dsound.dll");
    if(!audio_dll) { goto failure; }

    DirectSoundCreateFunc dsound_create = (DirectSoundCreateFunc) GetProcAddress(audio_dll, "DirectSoundCreate");
    if(!dsound_create) { goto failure; }

#define DEFAULT_AUDIO_DEVICE 0
    if(SUCCEEDED(dsound_create(DEFAULT_AUDIO_DEVICE, &direct_sound, NULL)))
    {
        SDL_SysWMinfo info = { };
        if(SDL_GetWindowWMInfo(window->sdl_window, &info))
        {
            direct_sound->SetCooperativeLevel(info.info.win.window, DSSCL_PRIORITY);
        }
        else
        {
            goto failure;
        }
    }

    return true;

failure:
    if(audio_dll)
    {
        FreeLibrary(audio_dll);
    }
    return false;
}

void ShutdownAudio()
{
    if(audio_dll)
    {
        FreeLibrary(audio_dll);
    }
}

WavFile* LoadWavFile(MemoryArena* arena, char* filename)
{
    uint32 file_size = 0;
    WavFile* result = (WavFile*)LoadDataFile(arena, &file_size, filename);

    if(!result || !file_size)
    {
        goto failure;
    }

    const size_t uncounted_wave_bytes = 8; // as per the file format
    size_t wave_file_size = uncounted_wave_bytes + result->riff_chunk.chunk_size;

    if(wave_file_size != file_size)
    {
        goto failure;
    }

    if(result->riff_chunk.chunk_id != CC_riff
            || result->fmt_chunk.subchunk1_id != CC_fmt
            || result->data_chunk.subchunk2_id != CC_data)
    {
        goto failure;
    }

    // Success!
    return result;

failure:
    if(result)
    {
        PopAllocation(arena, result);
    }
    return false;
}

struct AudioState
{

};




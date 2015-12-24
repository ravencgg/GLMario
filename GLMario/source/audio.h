#pragma once

#include "types.h"
#include "utility.h"

struct WavRiffChunk
{
    int32 chunk_id;
    int32 chunk_size;
    int32 format;
};

struct WavFmtChunk
{
    int32 subchunk1_id;
    int32 subchunk1_size;
    int16 audio_format;
    int16 num_channels;
    int32 sample_rate;
    int32 byte_rate;
    int16 block_align;
    int16 bits_per_sample;
};

struct WavDataChunk
{
    int32 subchunk2_id;
    int32 subchunk2_size;
    uint8 data[1]; // variable sized field;
};

struct WavFile
{
    WavRiffChunk riff_chunk;
    WavFmtChunk  fmt_chunk;
    WavDataChunk data_chunk;
};

enum class SoundType
{
    NONE,
    WAVE,
};

struct Window;
bool InitializeAudio();
void PauseAudio(bool pause);
bool LoadWavFile(const char* filename);


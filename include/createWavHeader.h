
#include <cstddef> // for size_t
#include <cstdint> // for uint8_t

// -----------------------------------------------------
// Create a standard 44-byte WAV header in the buffer
// Returns how many bytes were written to 'dest' (should be 44).
// -----------------------------------------------------
size_t createWavHeader(
    uint8_t *dest,
    size_t rawDataSize,
    uint32_t sampleRate,
    uint8_t bitsPerSample,
    uint8_t numChannels)
{
    // Reference: standard 44-byte WAV header layout
    // Chunk ID "RIFF"
    dest[0] = 'R';
    dest[1] = 'I';
    dest[2] = 'F';
    dest[3] = 'F';

    // Chunk size = 36 + rawDataSize
    uint32_t chunkSize = 36 + rawDataSize;
    dest[4] = (uint8_t)(chunkSize & 0xFF);
    dest[5] = (uint8_t)((chunkSize >> 8) & 0xFF);
    dest[6] = (uint8_t)((chunkSize >> 16) & 0xFF);
    dest[7] = (uint8_t)((chunkSize >> 24) & 0xFF);

    // Format "WAVE"
    dest[8] = 'W';
    dest[9] = 'A';
    dest[10] = 'V';
    dest[11] = 'E';

    // Sub-chunk 1 ID "fmt "
    dest[12] = 'f';
    dest[13] = 'm';
    dest[14] = 't';
    dest[15] = ' ';

    // Sub-chunk 1 size = 16 for PCM
    dest[16] = 16;
    dest[17] = 0;
    dest[18] = 0;
    dest[19] = 0;

    // Audio format = 1 (PCM)
    dest[20] = 1;
    dest[21] = 0;

    // Num channels
    dest[22] = numChannels;
    dest[23] = 0;

    // Sample rate
    dest[24] = (uint8_t)(sampleRate & 0xFF);
    dest[25] = (uint8_t)((sampleRate >> 8) & 0xFF);
    dest[26] = (uint8_t)((sampleRate >> 16) & 0xFF);
    dest[27] = (uint8_t)((sampleRate >> 24) & 0xFF);

    // Byte rate = sampleRate * numChannels * bitsPerSample/8
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    dest[28] = (uint8_t)(byteRate & 0xFF);
    dest[29] = (uint8_t)((byteRate >> 8) & 0xFF);
    dest[30] = (uint8_t)((byteRate >> 16) & 0xFF);
    dest[31] = (uint8_t)((byteRate >> 24) & 0xFF);

    // Block align = numChannels * bitsPerSample/8
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);
    dest[32] = (uint8_t)(blockAlign & 0xFF);
    dest[33] = (uint8_t)((blockAlign >> 8) & 0xFF);

    // Bits per sample
    dest[34] = bitsPerSample;
    dest[35] = 0;

    // Sub-chunk 2 ID "data"
    dest[36] = 'd';
    dest[37] = 'a';
    dest[38] = 't';
    dest[39] = 'a';

    // Sub-chunk 2 size = rawDataSize
    dest[40] = (uint8_t)(rawDataSize & 0xFF);
    dest[41] = (uint8_t)((rawDataSize >> 8) & 0xFF);
    dest[42] = (uint8_t)((rawDataSize >> 16) & 0xFF);
    dest[43] = (uint8_t)((rawDataSize >> 24) & 0xFF);

    return 44; // total header size
}
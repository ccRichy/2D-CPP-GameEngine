#include "xaudio2.h"
#include "my_types_keywords.h"
//#include "game.h"

#ifdef _XBOX 
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'

#else

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

#endif





struct WAV_Data
{
    //riff chunck
    uint32 riff_id;
    uint32 riff_chunk_size;
    uint32 format;

    //format chunk
    uint32 format_chunk_id;
    uint32 format_chunk_size;
    uint16 audio_format;
    uint16 num_channels;
    uint32 sample_rate;
    uint32 byte_rate;
    uint16 block_align;
    uint16 bits_per_sample;

    //data chunk
    uint8 data_chunk_id[4]; //stores the string literal
    uint32 data_chunk_size;
};
struct WAV_File
{
    WAV_Data data;
    char data_begin_point;
};

//#if 0
//WAV_File* load_wav(const char* path)
//{
//    int file_size = 0;
//    DEBUG_File file = DEBUG_platform_file_read_entire(path);
//    return (WAV_File*)file.memory;
//}
//#endif



struct Win32_XAudio_Data
{
    bool32 is_initialized;
    IXAudio2* object;
    WAVEFORMATEX wave_format;
};

struct Win32_XAudio_Voice : public IXAudio2VoiceCallback
{
    bool32 playing = false;
    IXAudio2SourceVoice* src_voice; //reference to its associated voice
    
    void OnStreamEnd() noexcept
    {
        src_voice->Stop();
        playing = false;
    }

    void OnBufferStart(void*)
    {
        playing = true;
    }

    void OnVoiceProcessingPassEnd() noexcept {}
    void OnVoiceProcessingPassStart(UINT32) noexcept {}
    void OnBufferEnd( void*) noexcept {}
    void OnLoopEnd( void*) noexcept {}
    void OnVoiceEnd( void*, HRESULT) noexcept {}
};

struct Win32_XAudio_Group
{
    IXAudio2SourceVoice* src_voice;
    Win32_XAudio_Voice* callback_voice;
};


globalvar Win32_XAudio_Group xaudio_voice_array[SND_MAX_CONCURRENT];



/////FUNCTIONS/////
HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;
    while (hr == S_OK)
    {
        DWORD dwRead;
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
        switch (dwChunkType)
        {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        default:
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                return HRESULT_FROM_WIN32(GetLastError());
        }
        dwOffset += sizeof(DWORD) * 2;
        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }
        dwOffset += dwChunkDataSize;
        if (bytesRead >= dwRIFFDataSize) return S_FALSE;
    }
    return S_OK;
}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwRead;
    if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}



internal Win32_XAudio_Data
win32_xaudio2_init()
{
    Win32_XAudio_Data data = {};
    data.is_initialized = true;
    //initializes COM
    if (FAILED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
        data.is_initialized = false;
    
    if (FAILED( XAudio2Create(&data.object, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        data.is_initialized = false;

    IXAudio2MasteringVoice* xaudio2_mastering_voice;
    if (FAILED(data.object->CreateMasteringVoice(
                   &xaudio2_mastering_voice, SND_CHANNELS, SND_SAMPLE_RATE, 0, 0, 0)))
        data.is_initialized = false;

    WAVEFORMATEX wave_format = {};
    wave_format.wFormatTag      = WAVE_FORMAT_PCM;
    wave_format.nChannels       = SND_CHANNELS;
    wave_format.wBitsPerSample  = SND_BITS_PER_SAMPLE;
    wave_format.nSamplesPerSec  = SND_SAMPLE_RATE;
    wave_format.nBlockAlign     = (SND_CHANNELS * SND_BITS_PER_SAMPLE) / 8;      
    wave_format.nAvgBytesPerSec = (SND_SAMPLE_RATE * wave_format.nBlockAlign);
    wave_format.cbSize          = 0;
    data.wave_format = wave_format;

    for (int voice_index = 0; voice_index < SND_MAX_CONCURRENT; ++voice_index)
    {
        Win32_XAudio_Group voices = xaudio_voice_array[voice_index];
        HRESULT hr = data.object->CreateSourceVoice(&voices.src_voice, &wave_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voices.callback_voice, nullptr, nullptr);
        voices.src_voice->SetVolume(0.5);
        if (FAILED(hr))
            data.is_initialized = false;
    }

    return data;
}


///UNDO HERE TO UNDO FUCKUPS
internal void DEBUG_xaudio2_play_sound(LPCSTR filename, Win32_XAudio_Data* data, void* sound_buffer)
{
    XAUDIO2_BUFFER xaudio2_buffer = {};
    IXAudio2SourceVoice* src_voice;

    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    if (file_handle)
    {
        DWORD dwChunkPosition;
        DWORD filetype;
        DWORD dwChunkSize;
        
        //check the file type, should be fourccWAVE or 'XWMA'
        FindChunk(file_handle, fourccRIFF, dwChunkSize, dwChunkPosition);
        ReadChunkData(file_handle, &filetype, sizeof(DWORD), dwChunkPosition);
       // if (filetype != fourccWAVE)
       //     return S_FALSE;

        FindChunk(file_handle, fourccFMT, dwChunkSize, dwChunkPosition);
        ReadChunkData(file_handle, &data->wave_format, SND_BUFFER_SIZE_BYTES, dwChunkPosition);

        FindChunk(file_handle, fourccDATA, dwChunkSize, dwChunkPosition);
        ReadChunkData(file_handle, sound_buffer, SND_BUFFER_SIZE_BYTES, dwChunkPosition);

        CloseHandle(file_handle);        
    }
    
    xaudio2_buffer.AudioBytes = SND_BUFFER_SIZE_BYTES;  //size of the audio buffer in bytes
    xaudio2_buffer.pAudioData = (const BYTE*)sound_buffer;  //buffer containing audio data
    xaudio2_buffer.Flags = XAUDIO2_END_OF_STREAM;
    
    HRESULT create_src_voice_result = data->object->CreateSourceVoice( &src_voice, &data->wave_format);
    
    HRESULT submit_src_buffer_result = src_voice->SubmitSourceBuffer(&xaudio2_buffer);
    if (S_OK == submit_src_buffer_result)
    {
        HRESULT sound_start_result = src_voice->Start(0);
        if (S_OK == sound_start_result)
        {
        }
    }
}



#if 0
int main()
{
    HRESULT hr;
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        cout << hr;

    IXAudio2* pXAudio2 = nullptr;
    if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        cout << hr;

    IXAudio2MasteringVoice* pMasterVoice = nullptr;
    if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
        cout << hr;

    WAVEFORMATEXTENSIBLE wfx = { 0 };
    XAUDIO2_BUFFER buffer = { 0 };

#ifdef _XBOX
    char* strFileName = "s.wav";
#else
    const TCHAR* strFileName = TEXT("s.wav");
#endif
    HANDLE hFile = CreateFile(
        strFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
        cout << HRESULT_FROM_WIN32(GetLastError());

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        cout << HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    //check the file type, should be fourccWAVE or 'XWMA'
    FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    DWORD filetype;
    ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
    if (filetype != fourccWAVE)
        return S_FALSE;

    FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

    FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
    BYTE* pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
    buffer.pAudioData = pDataBuffer;  //buffer containing audio data
    buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

    IXAudio2SourceVoice* pSourceVoice;
    if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx))) cout << hr;

    if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
        cout << hr;

    if (FAILED(hr = pSourceVoice->Start(0)))
        cout << hr;

    cout << "playing" << endl;
    cin.get();
}
#endif

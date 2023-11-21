#include "daisysp.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>


#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 64
#define TABLE_SIZE 200

using namespace daisysp;

Oscillator osc;


typedef struct
{
    char message[20];
}
paTestData;

static void checkErr(PaError err)
{
    if(err != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }
}



static int patestCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, 
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
)
{
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i;
    float osc_out;

    for(i=0; i<framesPerBuffer; i++)
    {
        osc_out = osc.Process();

        //set left and right
        *out++ = osc_out;
        *out++ = osc_out;
    }
    return 0;
}

int main()
{
    //printf("hello world\n");
    PaError err;
    PaStream* stream;
    paTestData data;
    PaStreamParameters outputParameters;

    //initialize oscillator
    osc.Init(44100.0f);
    osc.SetWaveform(osc.WAVE_POLYBLEP_SQUARE);
    osc.SetAmp(0.5f);
    osc.SetFreq(1000);

    err = Pa_Initialize();
    checkErr(err);

    memset(&outputParameters, 0, sizeof(outputParameters));

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if(outputParameters.device == paNoDevice)
    {
        printf("Error: No default device");
    }

    outputParameters.channelCount = 2;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowInputLatency;

    
    err = Pa_OpenStream(
        &stream, 
        NULL, //no input 
        &outputParameters,
        SAMPLE_RATE, 
        FRAMES_PER_BUFFER, 
        paNoFlag, 
        patestCallback, 
        &data
         );
    checkErr(err);
    
    sprintf( data.message, "No Message" );
    checkErr(err);

    err = Pa_StartStream(stream);
    checkErr(err);

    Pa_Sleep(10*1000);

    err = Pa_StopStream(stream);
    checkErr(err);

    err = Pa_CloseStream(stream);
    checkErr(err);

    err = Pa_Terminate();
    checkErr(err);
    
    return EXIT_SUCCESS;
}
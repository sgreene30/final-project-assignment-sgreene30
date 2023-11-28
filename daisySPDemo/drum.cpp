#include "daisysp.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>


#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 64
#define SLEEP_SEC 2

using namespace daisysp;

Oscillator osc;
AdEnv pitch_env;
AdEnv amplitude_env;


typedef struct
{
    PaStream* stream;
    bool triggered;
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

static void closePA(PaStream* stream)
{
    PaError err;
    err = Pa_StopStream(stream);
    checkErr(err);

    err = Pa_CloseStream(stream);
    checkErr(err);

    err = Pa_Terminate();
    checkErr(err);
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
    float osc_out, amp_out, pitch_out;

    if(!data->triggered)
    {
        pitch_env.Trigger();
        amplitude_env.Trigger();
        data->triggered = true;
    }

    for(i=0; i<framesPerBuffer; i++)
    {
        amp_out = amplitude_env.Process();
        pitch_out = pitch_env.Process();

        osc.SetAmp(amp_out);
        osc.SetFreq(pitch_out);
        
        osc_out = osc.Process();

        //set left and right
        *out++ = osc_out;
        *out++ = osc_out;
    }

    /*if(amplitude_env.GetCurrentSegment() == ADENV_SEG_IDLE)
    {
        printf("idle");
        closePA(data->stream);
    }*/

    return 0;
}

int main()
{
    //printf("hello world\n");
    PaError err;
    PaStream* stream;
    paTestData data;
    PaStreamParameters outputParameters;

    data.triggered = false;

    float sample_rate_f = (float)SAMPLE_RATE;

    //initialize oscillator
    osc.Init(sample_rate_f);
    osc.SetWaveform(osc.WAVE_POLYBLEP_TRI);
    osc.SetAmp(0.5f);
    osc.SetFreq(1000);

    //initialize pitch_env
    pitch_env.Init(sample_rate_f);
    pitch_env.SetTime(ADENV_SEG_ATTACK, 0.002);
    pitch_env.SetTime(ADENV_SEG_DECAY, 0.02);
    pitch_env.SetMax(400);
    pitch_env.SetMin(40);

    //initialize amplitude_env
    amplitude_env.Init(sample_rate_f);
    amplitude_env.SetTime(ADENV_SEG_ATTACK, 0.002);
    amplitude_env.SetTime(ADENV_SEG_DECAY, 0.5);
    amplitude_env.SetMax(1);
    amplitude_env.SetMin(0);
    //amplitude_env.SetCurve(-1);

    //init portaudio
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

    Pa_Sleep(SLEEP_SEC*1000);

    closePA(stream);
    
    return EXIT_SUCCESS;
}
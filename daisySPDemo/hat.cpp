#include "daisysp.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>


#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 128
#define SLEEP_SEC 0.5
#define LATENCY 0.05

using namespace daisysp;

WhiteNoise noise;
AdEnv amplitude_env;
//Svf filt;

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
    float amp_out, hat_out;

    if(!data->triggered)
    {
        amplitude_env.Trigger();
        data->triggered = true;
    }

    for(i=0; i<framesPerBuffer; i++)
    {
        amp_out = amplitude_env.Process();
        hat_out = noise.Process();
        //filt.Process(hat_out);

        hat_out *= amp_out;

        //set left and right
        *out++ = hat_out;
        *out++ = hat_out;
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

    float sample_rate_f = (float)SAMPLE_RATE;

    data.triggered = false;

    //initialize amplitude_env
    amplitude_env.Init(sample_rate_f);
    amplitude_env.SetTime(ADENV_SEG_ATTACK, 0.002);
    amplitude_env.SetTime(ADENV_SEG_DECAY, 0.015);
    amplitude_env.SetMax(1);
    amplitude_env.SetMin(0);
    //amplitude_env.SetCurve(-1);

    //initialize noise
    noise.Init();

    //initialize filter
    /*filt.Init(sample_rate_f);
    filt.SetFreq(5000.0);
    filt.SetRes(0.0);
    filt.SetDrive(0.5);*/

    //init portaudio
    err = Pa_Initialize();
    checkErr(err);

    memset(&outputParameters, 0, sizeof(outputParameters));

    //outputParameters.device = Pa_GetDefaultOutputDevice(); //default on host and maybe always HDMI on RaspberryPi
    outputParameters.device = 1; //headphone jack on raspberrypi

    if(outputParameters.device == paNoDevice)
    {
        printf("Error: No default device");
    }

    outputParameters.channelCount = 2;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = LATENCY;

    //printf("high latency: %f\n", Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency);
    //printf("low latency: %f\n", Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency);
    printf("default device: %d\n", Pa_GetDefaultOutputDevice());

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
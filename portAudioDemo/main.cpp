#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define TABLE_SIZE 200

typedef struct
{
    float ramp[TABLE_SIZE];
    int left_angle;
    int right_angle;
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

    for(i=0; i<framesPerBuffer; i++)
    {
        *out++ = data->ramp[data->left_angle];
        *out++ = data->ramp[data->right_angle];
        data->left_angle += 1;
        if(data->left_angle >= TABLE_SIZE)
        {
            data->left_angle -= TABLE_SIZE;
        }
        data->right_angle += 1;       
        if(data->right_angle >= TABLE_SIZE)
        {
            data->right_angle -= TABLE_SIZE;
        }
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

    //initialize ramp 
    for(int i = 0; i<TABLE_SIZE; i++)
    {
        data.ramp[i] = (float) (((float)i/((float)TABLE_SIZE)- 0.5)*0.25);
        //printf("%f, ", data.ramp[i]);
    }
    data.left_angle = data.right_angle = 0;

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
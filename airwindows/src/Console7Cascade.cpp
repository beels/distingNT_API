#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Console7Cascade"
#define AIRWINDOWS_DESCRIPTION "A drop-in replacement for Console7Channel that allows for MUCH higher gain."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','C','o','x' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	//Add your parameters here...
	kNumberOfParameters=1
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Cascade", .min = 0, .max = 1000, .def = 390, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		Float64 gainchase;
		Float64 chasespeed;
		double biquadA[11];
		double biquadB[11];
		double biquadC[11];
		double biquadD[11];
		double biquadE[11];
		uint32_t fpd;
	
	struct _dram {
		};
	_dram* dram;
};
_kernel kernels[1];

#include "../include/template2.h"
#include "../include/templateKernels.h"
void _airwindowsAlgorithm::_kernel::render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess ) {
#define inNumChannels (1)
{
	UInt32 nSampleFrames = inFramesToProcess;
	const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;
	
	double inputgain = 1.0-(pow(1.0-GetParameter( kParam_One ),2));
	//this happens to give us a boost factor where the track continues to get louder even
	//as it saturates and loses a bit of peak energy. Console7Channel channels go to 12! (.272,etc)
	//Neutral gain through the whole system with a full scale sine ia 0.772 on the gain knob
	if (gainchase != inputgain) chasespeed *= 2.0;
	if (chasespeed > inFramesToProcess) chasespeed = inFramesToProcess;
	if (gainchase < 0.0) gainchase = inputgain;
	
	biquadE[0] = biquadD[0] = biquadC[0] = biquadB[0] = biquadA[0] = 20000.0 / GetSampleRate();
	biquadA[1] = 3.19622661; //tenth order Butterworth out of five biquads
	biquadB[1] = 1.10134463; //the sharpness of the initial filter will produce more
	biquadC[1] = 0.70710678; //edge on initial stages of clipping
	biquadD[1] = 0.56116312;
    biquadE[1] = 0.50623256;
	
	double K = tan(M_PI * biquadA[0]); //lowpass
	double norm = 1.0 / (1.0 + K / biquadA[1] + K * K);
	biquadA[2] = K * K * norm;
	biquadA[3] = 2.0 * biquadA[2];
	biquadA[4] = biquadA[2];
	biquadA[5] = 2.0 * (K * K - 1.0) * norm;
	biquadA[6] = (1.0 - K / biquadA[1] + K * K) * norm;
	
	K = tan(M_PI * biquadA[0]);
	norm = 1.0 / (1.0 + K / biquadB[1] + K * K);
	biquadB[2] = K * K * norm;
	biquadB[3] = 2.0 * biquadB[2];
	biquadB[4] = biquadB[2];
	biquadB[5] = 2.0 * (K * K - 1.0) * norm;
	biquadB[6] = (1.0 - K / biquadB[1] + K * K) * norm;
	
	K = tan(M_PI * biquadC[0]);
	norm = 1.0 / (1.0 + K / biquadC[1] + K * K);
	biquadC[2] = K * K * norm;
	biquadC[3] = 2.0 * biquadC[2];
	biquadC[4] = biquadC[2];
	biquadC[5] = 2.0 * (K * K - 1.0) * norm;
	biquadC[6] = (1.0 - K / biquadC[1] + K * K) * norm;
	
	K = tan(M_PI * biquadD[0]);
	norm = 1.0 / (1.0 + K / biquadD[1] + K * K);
	biquadD[2] = K * K * norm;
	biquadD[3] = 2.0 * biquadD[2];
	biquadD[4] = biquadD[2];
	biquadD[5] = 2.0 * (K * K - 1.0) * norm;
	biquadD[6] = (1.0 - K / biquadD[1] + K * K) * norm;
	
	K = tan(M_PI * biquadE[0]);
	norm = 1.0 / (1.0 + K / biquadE[1] + K * K);
	biquadE[2] = K * K * norm;
	biquadE[3] = 2.0 * biquadE[2];
	biquadE[4] = biquadE[2];
	biquadE[5] = 2.0 * (K * K - 1.0) * norm;
	biquadE[6] = (1.0 - K / biquadE[1] + K * K) * norm;
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		
		double tempSample = biquadA[2]*inputSample+biquadA[3]*biquadA[7]+biquadA[4]*biquadA[8]-biquadA[5]*biquadA[9]-biquadA[6]*biquadA[10];
		biquadA[8] = biquadA[7]; biquadA[7] = inputSample; inputSample = tempSample; 
		biquadA[10] = biquadA[9]; biquadA[9] = inputSample; //DF1
		
		chasespeed *= 0.9999; chasespeed -= 0.01; if (chasespeed < 64.0) chasespeed = 64.0;
		//we have our chase speed compensated for recent fader activity
		gainchase = (((gainchase*chasespeed)+inputgain)/(chasespeed+1.0));
		//gainchase is chasing the target, as a simple multiply gain factor
		Float64 cascade = gainchase + (gainchase*0.62);
		
		if (1.0 != cascade) inputSample *= cascade;
		if (inputSample > 1.097) inputSample = 1.097;
		if (inputSample < -1.097) inputSample = -1.097;
		inputSample = ((sin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0) ?1:fabs(inputSample)))*0.8)+(sin(inputSample)*0.2);
		//Console7Channel distortion stage, with a simpler form of the gain boost: no extra accentuation, because it's repeated 5x
		
		tempSample = biquadB[2]*inputSample+biquadB[3]*biquadB[7]+biquadB[4]*biquadB[8]-biquadB[5]*biquadB[9]-biquadB[6]*biquadB[10];
		biquadB[8] = biquadB[7]; biquadB[7] = inputSample; inputSample = tempSample; 
		biquadB[10] = biquadB[9]; biquadB[9] = inputSample; //DF1
		
		if (1.0 != cascade) inputSample *= cascade;
		if (inputSample > 1.097) inputSample = 1.097;
		if (inputSample < -1.097) inputSample = -1.097;
		inputSample = ((sin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0) ?1:fabs(inputSample)))*0.8)+(sin(inputSample)*0.2);
		//Console7Channel distortion stage, with a simpler form of the gain boost: no extra accentuation, because it's repeated 5x
		
		tempSample = biquadC[2]*inputSample+biquadC[3]*biquadC[7]+biquadC[4]*biquadC[8]-biquadC[5]*biquadC[9]-biquadC[6]*biquadC[10];
		biquadC[8] = biquadC[7]; biquadC[7] = inputSample; inputSample = tempSample; 
		biquadC[10] = biquadC[9]; biquadC[9] = inputSample; //DF1
		
		if (1.0 != cascade) inputSample *= cascade;
		if (inputSample > 1.097) inputSample = 1.097;
		if (inputSample < -1.097) inputSample = -1.097;
		inputSample = ((sin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0) ?1:fabs(inputSample)))*0.8)+(sin(inputSample)*0.2);
		//Console7Channel distortion stage, with a simpler form of the gain boost: no extra accentuation, because it's repeated 5x
		
		tempSample = biquadD[2]*inputSample+biquadD[3]*biquadD[7]+biquadD[4]*biquadD[8]-biquadD[5]*biquadD[9]-biquadD[6]*biquadD[10];
		biquadD[8] = biquadD[7]; biquadD[7] = inputSample; inputSample = tempSample; 
		biquadD[10] = biquadD[9]; biquadD[9] = inputSample; //DF1
		
		if (1.0 != cascade) inputSample *= cascade;
		if (inputSample > 1.097) inputSample = 1.097;
		if (inputSample < -1.097) inputSample = -1.097;
		inputSample = ((sin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0) ?1:fabs(inputSample)))*0.8)+(sin(inputSample)*0.2);
		//Console7Channel distortion stage, with a simpler form of the gain boost: no extra accentuation, because it's repeated 5x
		
		tempSample = biquadE[2]*inputSample+biquadE[3]*biquadE[7]+biquadE[4]*biquadE[8]-biquadE[5]*biquadE[9]-biquadE[6]*biquadE[10];
		biquadE[8] = biquadE[7]; biquadE[7] = inputSample; inputSample = tempSample; 
		biquadE[10] = biquadE[9]; biquadE[9] = inputSample; //DF1
		
		if (1.0 != cascade) inputSample *= cascade;
		if (inputSample > 1.097) inputSample = 1.097;
		if (inputSample < -1.097) inputSample = -1.097;
		inputSample = ((sin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0) ?1:fabs(inputSample)))*0.8)+(sin(inputSample)*0.2);
		//Console7Channel distortion stage, with a simpler form of the gain boost: no extra accentuation, because it's repeated 5x
				
		if (cascade > 1.0) inputSample /= cascade;
		//we re-amplify after the distortion relative to how much we cut back previously.				
		
		//begin 32 bit floating point dither
		int expon; frexpf((float)inputSample, &expon);
		fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
		inputSample += ((double(fpd)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
		//end 32 bit floating point dither
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	gainchase = -1.0;
	chasespeed = 64.0;
	for (int x = 0; x < 11; x++) {biquadA[x] = 0.0;biquadB[x] = 0.0;biquadC[x] = 0.0;biquadD[x] = 0.0;biquadE[x] = 0.0;}
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

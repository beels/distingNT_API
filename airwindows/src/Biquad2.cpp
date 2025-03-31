#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Biquad2"
#define AIRWINDOWS_DESCRIPTION "The Airwindows biquad filter that's more sweepable and synthy."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','B','i','r' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	kParam_Five =4,
	//Add your parameters here...
	kNumberOfParameters=5
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, kParam2, kParam3, kParam4, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Type", .min = 1000, .max = 4000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Freq", .min = 3, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Q", .min = 100, .max = 5000, .def = 70, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL },
{ .name = "Output", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Inv/Dry/Wet", .min = -1000, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		double biquad[11];
		Float64 b[11];
		Float64 f[11];		
		Float64 frequencychase;
		Float64 resonancechase;
		Float64 outputchase;
		Float64 wetchase;
		Float64 frequencysetting;
		Float64 resonancesetting;
		Float64 outputsetting;
		Float64 wetsetting;
		Float64 chasespeed;
		uint32_t fpd;
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
	Float64 overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	
	int type = GetParameter( kParam_One);
	Float64 average = GetParameter( kParam_Two );
	Float64 frequencytarget = average*0.39; //biquad[0], goes to 1.0
	frequencytarget /= overallscale;
	if (frequencytarget < 0.0015/overallscale) frequencytarget = 0.0015/overallscale;
    Float64 resonancetarget = GetParameter( kParam_Three ); //biquad[1], goes to 50.0
	if (resonancetarget < 1.0) resonancetarget = 1.0;
	Float64 outputtarget = GetParameter( kParam_Four ); //scaled to res
	if (type < 3) outputtarget /= sqrt(resonancetarget);
	Float64 wettarget = GetParameter( kParam_Five ); //wet, goes -1.0 to 1.0
	
	//biquad contains these values:
	//[0] is frequency: 0.000001 to 0.499999 is near-zero to near-Nyquist
	//[1] is resonance, 0.7071 is Butterworth. Also can't be zero
	//[2] is a0 but you need distinct ones for additional biquad instances so it's here
	//[3] is a1 but you need distinct ones for additional biquad instances so it's here
	//[4] is a2 but you need distinct ones for additional biquad instances so it's here
	//[5] is b1 but you need distinct ones for additional biquad instances so it's here
	//[6] is b2 but you need distinct ones for additional biquad instances so it's here
	//[7] is a stored delayed sample (freq and res are stored so you can move them sample by sample)
	//[8] is a stored delayed sample (you have to include the coefficient making code if you do that)
	//[9] is a stored delayed sample (you have to include the coefficient making code if you do that)
	//[10] is a stored delayed sample (you have to include the coefficient making code if you do that)
	Float64 K = tan(M_PI * biquad[0]);
	Float64 norm = 1.0 / (1.0 + K / biquad[1] + K * K);
	//finished setting up biquad
	
	average = (1.0-average)*10.0; //max taps is 10, and low settings use more
	
	if (type == 1 || type == 3) average = 1.0;
	
	Float64 gain = average;
	if (gain > 1.0) {f[0] = 1.0; gain -= 1.0;} else {f[0] = gain; gain = 0.0;}
	if (gain > 1.0) {f[1] = 1.0; gain -= 1.0;} else {f[1] = gain; gain = 0.0;}
	if (gain > 1.0) {f[2] = 1.0; gain -= 1.0;} else {f[2] = gain; gain = 0.0;}
	if (gain > 1.0) {f[3] = 1.0; gain -= 1.0;} else {f[3] = gain; gain = 0.0;}
	if (gain > 1.0) {f[4] = 1.0; gain -= 1.0;} else {f[4] = gain; gain = 0.0;}
	if (gain > 1.0) {f[5] = 1.0; gain -= 1.0;} else {f[5] = gain; gain = 0.0;}
	if (gain > 1.0) {f[6] = 1.0; gain -= 1.0;} else {f[6] = gain; gain = 0.0;}
	if (gain > 1.0) {f[7] = 1.0; gain -= 1.0;} else {f[7] = gain; gain = 0.0;}
	if (gain > 1.0) {f[8] = 1.0; gain -= 1.0;} else {f[8] = gain; gain = 0.0;}
	if (gain > 1.0) {f[9] = 1.0; gain -= 1.0;} else {f[9] = gain; gain = 0.0;}
	//there, now we have a neat little moving average with remainders
	
	if (average < 1.0) average = 1.0;
	f[0] /= average;
	f[1] /= average;
	f[2] /= average;
	f[3] /= average;
	f[4] /= average;
	f[5] /= average;
	f[6] /= average;
	f[7] /= average;
	f[8] /= average;
	f[9] /= average;
	//and now it's neatly scaled, too
	//finished setting up average
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		double drySample = *sourceP;
		
		Float64 chasespeed = 50000;
		if (frequencychase < frequencytarget) chasespeed = 500000;
		chasespeed /= resonancechase;
		chasespeed *= overallscale;
		
		frequencychase = (((frequencychase*chasespeed)+frequencytarget)/(chasespeed+1.0));
		
		Float64 fasterchase = 1000 * overallscale;		
		resonancechase = (((resonancechase*fasterchase)+resonancetarget)/(fasterchase+1.0));
		outputchase = (((outputchase*fasterchase)+outputtarget)/(fasterchase+1.0));
		wetchase = (((wetchase*fasterchase)+wettarget)/(fasterchase+1.0));
		if (biquad[0] != frequencychase) {biquad[0] = frequencychase; K = tan(M_PI * biquad[0]);}
		if (biquad[1] != resonancechase) {biquad[1] = resonancechase; norm = 1.0 / (1.0 + K / biquad[1] + K * K);}
		
		if (type == 1) { //lowpass
			biquad[2] = K * K * norm;
			biquad[3] = 2.0 * biquad[2];
			biquad[4] = biquad[2];
			biquad[5] = 2.0 * (K * K - 1.0) * norm;
		}
		
		if (type == 2) { //highpass
			biquad[2] = norm;
			biquad[3] = -2.0 * biquad[2];
			biquad[4] = biquad[2];
			biquad[5] = 2.0 * (K * K - 1.0) * norm;
		}
		
		if (type == 3) { //bandpass
			biquad[2] = K / biquad[1] * norm;
			biquad[3] = 0.0; //bandpass can simplify the biquad kernel: leave out this multiply
			biquad[4] = -biquad[2];
			biquad[5] = 2.0 * (K * K - 1.0) * norm;
		}
		
		if (type == 4) { //notch
			biquad[2] = (1.0 + K * K) * norm;
			biquad[3] = 2.0 * (K * K - 1) * norm;
			biquad[4] = biquad[2];
			biquad[5] = biquad[3];
		}
		
		biquad[6] = (1.0 - K / biquad[1] + K * K) * norm;
				
		inputSample = sin(inputSample);
		//encode Console5: good cleanness
				
		double outSample = biquad[2]*inputSample+biquad[3]*biquad[7]+biquad[4]*biquad[8]-biquad[5]*biquad[9]-biquad[6]*biquad[10];
		biquad[8] = biquad[7]; biquad[7] = inputSample; inputSample = outSample; biquad[10] = biquad[9]; biquad[9] = inputSample; //DF1
				
		if (inputSample > 1.0) inputSample = 1.0;
		if (inputSample < -1.0) inputSample = -1.0;
			
		b[9] = b[8]; b[8] = b[7]; b[7] = b[6]; b[6] = b[5];
		b[5] = b[4]; b[4] = b[3]; b[3] = b[2]; b[2] = b[1];
		b[1] = b[0]; b[0] = inputSample;
		
		inputSample *= f[0];
		inputSample += (b[1] * f[1]);
		inputSample += (b[2] * f[2]);
		inputSample += (b[3] * f[3]);
		inputSample += (b[4] * f[4]);
		inputSample += (b[5] * f[5]);
		inputSample += (b[6] * f[6]);
		inputSample += (b[7] * f[7]);
		inputSample += (b[8] * f[8]);
		inputSample += (b[9] * f[9]); //intense averaging on deeper cutoffs
		
		if (inputSample > 1.0) inputSample = 1.0;
		if (inputSample < -1.0) inputSample = -1.0;
		//without this, you can get a NaN condition where it spits out DC offset at full blast!
		inputSample = asin(inputSample);
		//amplitude aspect
		if (inputSample > 1.0) inputSample = 1.0;
		if (inputSample < -1.0) inputSample = -1.0;
		//and then Console5 will spit out overs if you let it
		
		if (outputchase < 1.0) {
			inputSample *= outputchase;
		}
		
		if (wetchase < 1.0) {
			inputSample = (inputSample*wetchase) + (drySample*(1.0-fabs(wetchase)));
			//inv/dry/wet lets us turn LP into HP and band into notch
		}
		
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
	for (int x = 0; x < 11; x++) {biquad[x] = 0.0; b[x] = 0.0; f[x] = 0.0;}
	frequencychase = 0.0015;
	resonancechase = 0.001;
	outputchase = 1.0;
	wetchase = 1.0;
	
	frequencysetting = -1.0;
	resonancesetting = -1.0;
	outputsetting = -1.0;
	wetsetting = -2.0; //-1.0 is a possible setting here and this forces an update on chasespeed
	
	chasespeed = 500.0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

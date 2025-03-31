#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "ZHighpass2"
#define AIRWINDOWS_DESCRIPTION "Acts more like the Emu e6400 Ultra highpass in motion, with coefficient interpolation."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','Z','H','j' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	//Add your parameters here...
	kNumberOfParameters=4
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, kParam2, kParam3, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Input", .min = 0, .max = 1000, .def = 100, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Freq", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Output", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Poles", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		
		double iirSampleA;
		enum {
			biq_freq,
			biq_reso,
			biq_a0,
			biq_a1,
			biq_a2,
			biq_b1,
			biq_b2,
			biq_aA0,
			biq_aA1,
			biq_aA2,
			biq_bA1,
			biq_bA2,
			biq_aB0,
			biq_aB1,
			biq_aB2,
			biq_bB1,
			biq_bB2,
			biq_sL1,
			biq_sL2,
			biq_sR1,
			biq_sR2,
			biq_total
		}; //coefficient interpolating biquad filter, stereo
		double inTrimA;
		double inTrimB;
		double outTrimA;
		double outTrimB;
		double wetA;
		double wetB;
		
		enum {
			fix_freq,
			fix_reso,
			fix_a0,
			fix_a1,
			fix_a2,
			fix_b1,
			fix_b2,
			fix_sL1,
			fix_sL2,
			fix_sR1,
			fix_sR2,
			fix_total
		}; //fixed frequency biquad filter for ultrasonics, stereo
		
		uint32_t fpd;
	
	struct _dram {
			double biquadA[biq_total];
		double biquadB[biq_total];
		double biquadC[biq_total];
		double biquadD[biq_total];
		double fixA[fix_total];
		double fixB[fix_total];
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
	double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	
	dram->biquadA[biq_freq] = ((pow(GetParameter( kParam_Two ),4)*9500.0)/GetSampleRate())+0.00076;
	//double clipFactor = 1.212-((1.0-GetParameter( kParam_Two ))*0.496);
	dram->biquadA[biq_reso] = 1.0;
	dram->biquadA[biq_aA0] = dram->biquadA[biq_aB0];
	dram->biquadA[biq_aA1] = dram->biquadA[biq_aB1];
	dram->biquadA[biq_aA2] = dram->biquadA[biq_aB2];
	dram->biquadA[biq_bA1] = dram->biquadA[biq_bB1];
	dram->biquadA[biq_bA2] = dram->biquadA[biq_bB2];
	//previous run through the buffer is still in the filter, so we move it
	//to the A section and now it's the new starting point.	
	double K = tan(M_PI * dram->biquadA[biq_freq]);
	double norm = 1.0 / (1.0 + K / dram->biquadA[biq_reso] + K * K);
	dram->biquadA[biq_aB0] = norm;
	dram->biquadA[biq_aB1] = -2.0 * dram->biquadA[biq_aB0];
	dram->biquadA[biq_aB2] = dram->biquadA[biq_aB0];
	dram->biquadA[biq_bB1] = 2.0 * (K * K - 1.0) * norm;
	dram->biquadA[biq_bB2] = (1.0 - K / dram->biquadA[biq_reso] + K * K) * norm;
	
	//opamp stuff
	inTrimA = inTrimB;
	inTrimB = GetParameter( kParam_One )*10.0;
	inTrimB *= inTrimB; inTrimB *= inTrimB;
	outTrimA = outTrimB;
	outTrimB = GetParameter( kParam_Three )*10.0;
	wetA = wetB;
	wetB = pow(GetParameter( kParam_Four ),2);
	
	double iirAmountA = 0.00069/overallscale;
	dram->fixA[fix_freq] = dram->fixB[fix_freq] = 15500.0 / GetSampleRate();
    dram->fixA[fix_reso] = dram->fixB[fix_reso] = 0.935;
	K = tan(M_PI * dram->fixB[fix_freq]); //lowpass
	norm = 1.0 / (1.0 + K / dram->fixB[fix_reso] + K * K);
	dram->fixA[fix_a0] = dram->fixB[fix_a0] = K * K * norm;
	dram->fixA[fix_a1] = dram->fixB[fix_a1] = 2.0 * dram->fixB[fix_a0];
	dram->fixA[fix_a2] = dram->fixB[fix_a2] = dram->fixB[fix_a0];
	dram->fixA[fix_b1] = dram->fixB[fix_b1] = 2.0 * (K * K - 1.0) * norm;
	dram->fixA[fix_b2] = dram->fixB[fix_b2] = (1.0 - K / dram->fixB[fix_reso] + K * K) * norm;
	//end opamp stuff	
	
 	double trim = 0.1+(3.712*dram->biquadA[biq_freq]);
	double outSample = 0.0;
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		double drySample = *sourceP;
		double overallDrySample = *sourceP;
		
		double temp = (double)nSampleFrames/inFramesToProcess;
		dram->biquadA[biq_a0] = (dram->biquadA[biq_aA0]*temp)+(dram->biquadA[biq_aB0]*(1.0-temp));
		dram->biquadA[biq_a1] = (dram->biquadA[biq_aA1]*temp)+(dram->biquadA[biq_aB1]*(1.0-temp));
		dram->biquadA[biq_a2] = (dram->biquadA[biq_aA2]*temp)+(dram->biquadA[biq_aB2]*(1.0-temp));
		dram->biquadA[biq_b1] = (dram->biquadA[biq_bA1]*temp)+(dram->biquadA[biq_bB1]*(1.0-temp));
		dram->biquadA[biq_b2] = (dram->biquadA[biq_bA2]*temp)+(dram->biquadA[biq_bB2]*(1.0-temp));
		for (int x = 0; x < 7; x++) {dram->biquadD[x] = dram->biquadC[x] = dram->biquadB[x] = dram->biquadA[x];}
		//this is the interpolation code for the biquad
		double inTrim = (inTrimA*temp)+(inTrimB*(1.0-temp));
		double outTrim = (outTrimA*temp)+(outTrimB*(1.0-temp));
		double wet = (wetA*temp)+(wetB*(1.0-temp));
		double aWet = 1.0;
		double bWet = 1.0;
		double cWet = 1.0;
		double dWet = wet*4.0;
		//four-stage wet/dry control using progressive stages that bypass when not engaged
		if (dWet < 1.0) {aWet = dWet; bWet = 0.0; cWet = 0.0; dWet = 0.0;}
		else if (dWet < 2.0) {bWet = dWet - 1.0; cWet = 0.0; dWet = 0.0;}
		else if (dWet < 3.0) {cWet = dWet - 2.0; dWet = 0.0;}
		else {dWet -= 3.0;}
		//this is one way to make a little set of dry/wet stages that are successively added to the
		//output as the control is turned up. Each one independently goes from 0-1 and stays at 1
		//beyond that point: this is a way to progressively add a 'black box' sound processing
		//which lets you fall through to simpler processing at lower settings.
		
		if (inTrim != 1.0) inputSample *= inTrim;
		if (inputSample > 1.0) inputSample = 1.0; if (inputSample < -1.0) inputSample = -1.0;
		inputSample *= trim;
		//inputSample /= clipFactor;
		outSample = (inputSample * dram->biquadA[biq_a0]) + dram->biquadA[biq_sL1];
		if (outSample > 1.0) outSample = 1.0; if (outSample < -1.0) outSample = -1.0;
		dram->biquadA[biq_sL1] = (inputSample * dram->biquadA[biq_a1]) - (outSample * dram->biquadA[biq_b1]) + dram->biquadA[biq_sL2];
		dram->biquadA[biq_sL2] = (inputSample * dram->biquadA[biq_a2]) - (outSample * dram->biquadA[biq_b2]);
		drySample = inputSample = outSample;
		
		if (bWet > 0.0) {
			//inputSample /= clipFactor;
			outSample = (inputSample * dram->biquadB[biq_a0]) + dram->biquadB[biq_sL1];
			if (outSample > 1.0) outSample = 1.0; if (outSample < -1.0) outSample = -1.0;
			dram->biquadB[biq_sL1] = (inputSample * dram->biquadB[biq_a1]) - (outSample * dram->biquadB[biq_b1]) + dram->biquadB[biq_sL2];
			dram->biquadB[biq_sL2] = (inputSample * dram->biquadB[biq_a2]) - (outSample * dram->biquadB[biq_b2]);
			drySample = inputSample = (outSample * bWet) + (drySample * (1.0-bWet));
		}
		if (cWet > 0.0) {
			//inputSample /= clipFactor;
			outSample = (inputSample * dram->biquadC[biq_a0]) + dram->biquadC[biq_sL1];
			if (outSample > 1.0) outSample = 1.0; if (outSample < -1.0) outSample = -1.0;
			dram->biquadC[biq_sL1] = (inputSample * dram->biquadC[biq_a1]) - (outSample * dram->biquadC[biq_b1]) + dram->biquadC[biq_sL2];
			dram->biquadC[biq_sL2] = (inputSample * dram->biquadC[biq_a2]) - (outSample * dram->biquadC[biq_b2]);
			drySample = inputSample = (outSample * cWet) + (drySample * (1.0-cWet));
		}
		if (dWet > 0.0) {
			//inputSample /= clipFactor;
			outSample = (inputSample * dram->biquadD[biq_a0]) + dram->biquadD[biq_sL1];
			if (outSample > 1.0) outSample = 1.0; if (outSample < -1.0) outSample = -1.0;
			dram->biquadD[biq_sL1] = (inputSample * dram->biquadD[biq_a1]) - (outSample * dram->biquadD[biq_b1]) + dram->biquadD[biq_sL2];
			dram->biquadD[biq_sL2] = (inputSample * dram->biquadD[biq_a2]) - (outSample * dram->biquadD[biq_b2]);
			drySample = inputSample = (outSample * dWet) + (drySample * (1.0-dWet));
		}
		
		//inputSample /= clipFactor;
		
		//opamp stage
		if (fabs(iirSampleA)<1.18e-37) iirSampleA = 0.0;
		iirSampleA = (iirSampleA * (1.0 - iirAmountA)) + (inputSample * iirAmountA);
		inputSample -= iirSampleA;
		
		outSample = (inputSample * dram->fixA[fix_a0]) + dram->fixA[fix_sL1];
		dram->fixA[fix_sL1] = (inputSample * dram->fixA[fix_a1]) - (outSample * dram->fixA[fix_b1]) + dram->fixA[fix_sL2];
		dram->fixA[fix_sL2] = (inputSample * dram->fixA[fix_a2]) - (outSample * dram->fixA[fix_b2]);
		inputSample = outSample; //fixed biquad filtering ultrasonics
		
		if (inputSample > 1.0) inputSample = 1.0; if (inputSample < -1.0) inputSample = -1.0;
		inputSample -= (inputSample*inputSample*inputSample*inputSample*inputSample*0.1768);
		
		outSample = (inputSample * dram->fixB[fix_a0]) + dram->fixB[fix_sL1];
		dram->fixB[fix_sL1] = (inputSample * dram->fixB[fix_a1]) - (outSample * dram->fixB[fix_b1]) + dram->fixB[fix_sL2];
		dram->fixB[fix_sL2] = (inputSample * dram->fixB[fix_a2]) - (outSample * dram->fixB[fix_b2]);
		inputSample = outSample; //fixed biquad filtering ultrasonics
		
		if (outTrim != 1.0) inputSample *= outTrim;		
		//end opamp stage
		
		if (aWet !=1.0) {
			inputSample = (inputSample * aWet) + (overallDrySample * (1.0-aWet));
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
	iirSampleA = 0.0;
	for (int x = 0; x < biq_total; x++) {dram->biquadA[x] = 0.0; dram->biquadB[x] = 0.0; dram->biquadC[x] = 0.0; dram->biquadD[x] = 0.0;}
	inTrimA = 0.1; inTrimB = 0.1;
	outTrimA = 1.0; outTrimB = 1.0;
	wetA = 0.5; wetB = 0.5;
	for (int x = 0; x < fix_total; x++) {dram->fixA[x] = 0.0; dram->fixB[x] = 0.0;}
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

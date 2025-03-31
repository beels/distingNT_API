#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Parametric"
#define AIRWINDOWS_DESCRIPTION "Three bands of ConsoleX EQ in advance."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','P','a','r' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_TRF =0,
	kParam_TRG =1,
	kParam_TRR =2,
	kParam_HMF =3,
	kParam_HMG =4,
	kParam_HMR =5,
	kParam_LMF =6,
	kParam_LMG =7,
	kParam_LMR =8,
	kParam_DW = 9,
	//Add your parameters here...
	kNumberOfParameters=10
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, kParam8, kParam9, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Tr Freq", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Treble", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Tr Reso", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "HM Freq", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "HighMid", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "HM Reso", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "LM Freq", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "LowMid", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "LM Reso", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, kParam8, kParam9, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		
		enum { 
			biqs_freq, biqs_reso, biqs_level,
			biqs_nonlin, biqs_temp, biqs_dis,
			biqs_a0, biqs_a1, biqs_b1, biqs_b2,
			biqs_c0, biqs_c1, biqs_d1, biqs_d2,
			biqs_e0, biqs_e1, biqs_f1, biqs_f2,
			biqs_aL1, biqs_aL2, biqs_aR1, biqs_aR2,
			biqs_cL1, biqs_cL2, biqs_cR1, biqs_cR2,
			biqs_eL1, biqs_eL2, biqs_eR1, biqs_eR2,
			biqs_outL, biqs_outR, biqs_total
		};
		double high[biqs_total];
		double hmid[biqs_total];
		double lmid[biqs_total];
		
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
	double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	
	high[biqs_freq] = (((pow(GetParameter( kParam_TRF ),3)*14500.0)+1500.0)/GetSampleRate());
	if (high[biqs_freq] < 0.0001) high[biqs_freq] = 0.0001;
	high[biqs_nonlin] = GetParameter( kParam_TRG );
	high[biqs_level] = (high[biqs_nonlin]*2.0)-1.0;
	if (high[biqs_level] > 0.0) high[biqs_level] *= 2.0;
	high[biqs_reso] = ((0.5+(high[biqs_nonlin]*0.5)+sqrt(high[biqs_freq]))-(1.0-pow(1.0-GetParameter( kParam_TRR ),2.0)))+0.5+(high[biqs_nonlin]*0.5);
	double K = tan(M_PI * high[biqs_freq]);
	double norm = 1.0 / (1.0 + K / (high[biqs_reso]*1.93185165) + K * K);
	high[biqs_a0] = K / (high[biqs_reso]*1.93185165) * norm;
	high[biqs_b1] = 2.0 * (K * K - 1.0) * norm;
	high[biqs_b2] = (1.0 - K / (high[biqs_reso]*1.93185165) + K * K) * norm;
	norm = 1.0 / (1.0 + K / (high[biqs_reso]*0.70710678) + K * K);
	high[biqs_c0] = K / (high[biqs_reso]*0.70710678) * norm;
	high[biqs_d1] = 2.0 * (K * K - 1.0) * norm;
	high[biqs_d2] = (1.0 - K / (high[biqs_reso]*0.70710678) + K * K) * norm;
	norm = 1.0 / (1.0 + K / (high[biqs_reso]*0.51763809) + K * K);
	high[biqs_e0] = K / (high[biqs_reso]*0.51763809) * norm;
	high[biqs_f1] = 2.0 * (K * K - 1.0) * norm;
	high[biqs_f2] = (1.0 - K / (high[biqs_reso]*0.51763809) + K * K) * norm;
	//high
	
	hmid[biqs_freq] = (((pow(GetParameter( kParam_HMF ),3)*6400.0)+600.0)/GetSampleRate());
	if (hmid[biqs_freq] < 0.0001) hmid[biqs_freq] = 0.0001;
	hmid[biqs_nonlin] = GetParameter( kParam_HMG );
	hmid[biqs_level] = (hmid[biqs_nonlin]*2.0)-1.0;
	if (hmid[biqs_level] > 0.0) hmid[biqs_level] *= 2.0;
	hmid[biqs_reso] = ((0.5+(hmid[biqs_nonlin]*0.5)+sqrt(hmid[biqs_freq]))-(1.0-pow(1.0-GetParameter( kParam_HMR ),2.0)))+0.5+(hmid[biqs_nonlin]*0.5);
	K = tan(M_PI * hmid[biqs_freq]);
	norm = 1.0 / (1.0 + K / (hmid[biqs_reso]*1.93185165) + K * K);
	hmid[biqs_a0] = K / (hmid[biqs_reso]*1.93185165) * norm;
	hmid[biqs_b1] = 2.0 * (K * K - 1.0) * norm;
	hmid[biqs_b2] = (1.0 - K / (hmid[biqs_reso]*1.93185165) + K * K) * norm;
	norm = 1.0 / (1.0 + K / (hmid[biqs_reso]*0.70710678) + K * K);
	hmid[biqs_c0] = K / (hmid[biqs_reso]*0.70710678) * norm;
	hmid[biqs_d1] = 2.0 * (K * K - 1.0) * norm;
	hmid[biqs_d2] = (1.0 - K / (hmid[biqs_reso]*0.70710678) + K * K) * norm;
	norm = 1.0 / (1.0 + K / (hmid[biqs_reso]*0.51763809) + K * K);
	hmid[biqs_e0] = K / (hmid[biqs_reso]*0.51763809) * norm;
	hmid[biqs_f1] = 2.0 * (K * K - 1.0) * norm;
	hmid[biqs_f2] = (1.0 - K / (hmid[biqs_reso]*0.51763809) + K * K) * norm;
	//hmid
	
	lmid[biqs_freq] = (((pow(GetParameter( kParam_LMF ),3)*2200.0)+20.0)/GetSampleRate());
	if (lmid[biqs_freq] < 0.00001) lmid[biqs_freq] = 0.00001;
	lmid[biqs_nonlin] = GetParameter( kParam_LMG );
	lmid[biqs_level] = (lmid[biqs_nonlin]*2.0)-1.0;
	if (lmid[biqs_level] > 0.0) lmid[biqs_level] *= 2.0;
	lmid[biqs_reso] = ((0.5+(lmid[biqs_nonlin]*0.5)+sqrt(lmid[biqs_freq]))-(1.0-pow(1.0-GetParameter( kParam_LMR ),2.0)))+0.5+(lmid[biqs_nonlin]*0.5);
	K = tan(M_PI * lmid[biqs_freq]);
	norm = 1.0 / (1.0 + K / (lmid[biqs_reso]*1.93185165) + K * K);
	lmid[biqs_a0] = K / (lmid[biqs_reso]*1.93185165) * norm;
	lmid[biqs_b1] = 2.0 * (K * K - 1.0) * norm;
	lmid[biqs_b2] = (1.0 - K / (lmid[biqs_reso]*1.93185165) + K * K) * norm;
	norm = 1.0 / (1.0 + K / (lmid[biqs_reso]*0.70710678) + K * K);
	lmid[biqs_c0] = K / (lmid[biqs_reso]*0.70710678) * norm;
	lmid[biqs_d1] = 2.0 * (K * K - 1.0) * norm;
	lmid[biqs_d2] = (1.0 - K / (lmid[biqs_reso]*0.70710678) + K * K) * norm;
	norm = 1.0 / (1.0 + K / (lmid[biqs_reso]*0.51763809) + K * K);
	lmid[biqs_e0] = K / (lmid[biqs_reso]*0.51763809) * norm;
	lmid[biqs_f1] = 2.0 * (K * K - 1.0) * norm;
	lmid[biqs_f2] = (1.0 - K / (lmid[biqs_reso]*0.51763809) + K * K) * norm;
	//lmid
		
	double wet = GetParameter( kParam_DW );
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		
		//begin Stacked Biquad With Reversed Neutron Flow L
		high[biqs_outL] = inputSample * fabs(high[biqs_level]);
		high[biqs_dis] = fabs(high[biqs_a0] * (1.0+(high[biqs_outL]*high[biqs_nonlin])));
		if (high[biqs_dis] > 1.0) high[biqs_dis] = 1.0;
		high[biqs_temp] = (high[biqs_outL] * high[biqs_dis]) + high[biqs_aL1];
		high[biqs_aL1] = high[biqs_aL2] - (high[biqs_temp]*high[biqs_b1]);
		high[biqs_aL2] = (high[biqs_outL] * -high[biqs_dis]) - (high[biqs_temp]*high[biqs_b2]);
		high[biqs_outL] = high[biqs_temp];
		high[biqs_dis] = fabs(high[biqs_c0] * (1.0+(high[biqs_outL]*high[biqs_nonlin])));
		if (high[biqs_dis] > 1.0) high[biqs_dis] = 1.0;
		high[biqs_temp] = (high[biqs_outL] * high[biqs_dis]) + high[biqs_cL1];
		high[biqs_cL1] = high[biqs_cL2] - (high[biqs_temp]*high[biqs_d1]);
		high[biqs_cL2] = (high[biqs_outL] * -high[biqs_dis]) - (high[biqs_temp]*high[biqs_d2]);
		high[biqs_outL] = high[biqs_temp];
		high[biqs_dis] = fabs(high[biqs_e0] * (1.0+(high[biqs_outL]*high[biqs_nonlin])));
		if (high[biqs_dis] > 1.0) high[biqs_dis] = 1.0;
		high[biqs_temp] = (high[biqs_outL] * high[biqs_dis]) + high[biqs_eL1];
		high[biqs_eL1] = high[biqs_eL2] - (high[biqs_temp]*high[biqs_f1]);
		high[biqs_eL2] = (high[biqs_outL] * -high[biqs_dis]) - (high[biqs_temp]*high[biqs_f2]);
		high[biqs_outL] = high[biqs_temp]; high[biqs_outL] *= high[biqs_level];
		if (high[biqs_level] > 1.0) high[biqs_outL] *= high[biqs_level];
		//end Stacked Biquad With Reversed Neutron Flow L
		
		//begin Stacked Biquad With Reversed Neutron Flow L
		hmid[biqs_outL] = inputSample * fabs(hmid[biqs_level]);
		hmid[biqs_dis] = fabs(hmid[biqs_a0] * (1.0+(hmid[biqs_outL]*hmid[biqs_nonlin])));
		if (hmid[biqs_dis] > 1.0) hmid[biqs_dis] = 1.0;
		hmid[biqs_temp] = (hmid[biqs_outL] * hmid[biqs_dis]) + hmid[biqs_aL1];
		hmid[biqs_aL1] = hmid[biqs_aL2] - (hmid[biqs_temp]*hmid[biqs_b1]);
		hmid[biqs_aL2] = (hmid[biqs_outL] * -hmid[biqs_dis]) - (hmid[biqs_temp]*hmid[biqs_b2]);
		hmid[biqs_outL] = hmid[biqs_temp];
		hmid[biqs_dis] = fabs(hmid[biqs_c0] * (1.0+(hmid[biqs_outL]*hmid[biqs_nonlin])));
		if (hmid[biqs_dis] > 1.0) hmid[biqs_dis] = 1.0;
		hmid[biqs_temp] = (hmid[biqs_outL] * hmid[biqs_dis]) + hmid[biqs_cL1];
		hmid[biqs_cL1] = hmid[biqs_cL2] - (hmid[biqs_temp]*hmid[biqs_d1]);
		hmid[biqs_cL2] = (hmid[biqs_outL] * -hmid[biqs_dis]) - (hmid[biqs_temp]*hmid[biqs_d2]);
		hmid[biqs_outL] = hmid[biqs_temp];
		hmid[biqs_dis] = fabs(hmid[biqs_e0] * (1.0+(hmid[biqs_outL]*hmid[biqs_nonlin])));
		if (hmid[biqs_dis] > 1.0) hmid[biqs_dis] = 1.0;
		hmid[biqs_temp] = (hmid[biqs_outL] * hmid[biqs_dis]) + hmid[biqs_eL1];
		hmid[biqs_eL1] = hmid[biqs_eL2] - (hmid[biqs_temp]*hmid[biqs_f1]);
		hmid[biqs_eL2] = (hmid[biqs_outL] * -hmid[biqs_dis]) - (hmid[biqs_temp]*hmid[biqs_f2]);
		hmid[biqs_outL] = hmid[biqs_temp]; hmid[biqs_outL] *= hmid[biqs_level];
		if (hmid[biqs_level] > 1.0) hmid[biqs_outL] *= hmid[biqs_level];
		//end Stacked Biquad With Reversed Neutron Flow L
		
		//begin Stacked Biquad With Reversed Neutron Flow L
		lmid[biqs_outL] = inputSample * fabs(lmid[biqs_level]);
		lmid[biqs_dis] = fabs(lmid[biqs_a0] * (1.0+(lmid[biqs_outL]*lmid[biqs_nonlin])));
		if (lmid[biqs_dis] > 1.0) lmid[biqs_dis] = 1.0;
		lmid[biqs_temp] = (lmid[biqs_outL] * lmid[biqs_dis]) + lmid[biqs_aL1];
		lmid[biqs_aL1] = lmid[biqs_aL2] - (lmid[biqs_temp]*lmid[biqs_b1]);
		lmid[biqs_aL2] = (lmid[biqs_outL] * -lmid[biqs_dis]) - (lmid[biqs_temp]*lmid[biqs_b2]);
		lmid[biqs_outL] = lmid[biqs_temp];
		lmid[biqs_dis] = fabs(lmid[biqs_c0] * (1.0+(lmid[biqs_outL]*lmid[biqs_nonlin])));
		if (lmid[biqs_dis] > 1.0) lmid[biqs_dis] = 1.0;
		lmid[biqs_temp] = (lmid[biqs_outL] * lmid[biqs_dis]) + lmid[biqs_cL1];
		lmid[biqs_cL1] = lmid[biqs_cL2] - (lmid[biqs_temp]*lmid[biqs_d1]);
		lmid[biqs_cL2] = (lmid[biqs_outL] * -lmid[biqs_dis]) - (lmid[biqs_temp]*lmid[biqs_d2]);
		lmid[biqs_outL] = lmid[biqs_temp];
		lmid[biqs_dis] = fabs(lmid[biqs_e0] * (1.0+(lmid[biqs_outL]*lmid[biqs_nonlin])));
		if (lmid[biqs_dis] > 1.0) lmid[biqs_dis] = 1.0;
		lmid[biqs_temp] = (lmid[biqs_outL] * lmid[biqs_dis]) + lmid[biqs_eL1];
		lmid[biqs_eL1] = lmid[biqs_eL2] - (lmid[biqs_temp]*lmid[biqs_f1]);
		lmid[biqs_eL2] = (lmid[biqs_outL] * -lmid[biqs_dis]) - (lmid[biqs_temp]*lmid[biqs_f2]);
		lmid[biqs_outL] = lmid[biqs_temp]; lmid[biqs_outL] *= lmid[biqs_level];
		if (lmid[biqs_level] > 1.0) lmid[biqs_outL] *= lmid[biqs_level];
		//end Stacked Biquad With Reversed Neutron Flow L
				
		double parametric = high[biqs_outL] + hmid[biqs_outL] + lmid[biqs_outL];
		inputSample += (parametric * wet); //purely a parallel filter stage here
		
		
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
	for (int x = 0; x < biqs_total; x++) {
		high[x] = 0.0;
		hmid[x] = 0.0;
		lmid[x] = 0.0;
	}
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

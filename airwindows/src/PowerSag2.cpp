#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "PowerSag2"
#define AIRWINDOWS_DESCRIPTION "My improved circuit-starve plugin, now with inverse effect!"
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','P','o','x' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	//Add your parameters here...
	kNumberOfParameters=2
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Range", .min = 0, .max = 1000, .def = 300, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Inv/Dry/Wet", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;

		Float64 control;
		int gcount;		
		uint32_t fpd;
	
	struct _dram {
			Float64 d[16386];
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

	Float64 depth = pow(GetParameter( kParam_One ),4);
	int offset = (int)(depth * 16383) + 1;	
	Float64 wet = GetParameter( kParam_Two );

	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		Float64 drySample = inputSample;

		if (gcount < 0 || gcount > 16384) {gcount = 16384;}		
		dram->d[gcount] = fabs(inputSample);
		control += dram->d[gcount];
		control -= dram->d[gcount+offset-((gcount+offset > 16384)?16384:0)];
		gcount--;
		
		if (control > offset) control = offset; if (control < 0.0) control = 0.0;				
		
		Float64 burst = inputSample * (control / sqrt(offset));
		Float64 clamp = inputSample / ((burst == 0.0)?1.0:burst);
		
		if (clamp > 1.0) clamp = 1.0; if (clamp < 0.0) clamp = 0.0;
		inputSample *= clamp;
		Float64 difference = drySample - inputSample;
		if (wet < 0.0) drySample *= (wet+1.0);
		inputSample = drySample - (difference * wet);
		
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
	for(int count = 0; count < 16385; count++) {dram->d[count] = 0;}
	control = 0;
	gcount = 0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

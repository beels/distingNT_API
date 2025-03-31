#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Sweeten"
#define AIRWINDOWS_DESCRIPTION "Where you can find super-clean second harmonic."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','S','w','f' )
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
{ .name = "Sweeten", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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

		double savg[9];
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
	int cycleEnd = floor(overallscale);
	if (cycleEnd < 1) cycleEnd = 1;
	if (cycleEnd > 4) cycleEnd = 4;
	//this is going to be 2 for 88.1 or 96k, 3 for silly people, 4 for 176 or 192k
	
	int sweetBits = 10-floor(GetParameter( kParam_One )*10.0);
	double sweet = 1.0;
	switch (sweetBits)
	{
		case 11: sweet = 0.00048828125; break;
		case 10: sweet = 0.0009765625; break;
		case 9: sweet = 0.001953125; break;
		case 8: sweet = 0.00390625; break;
		case 7: sweet = 0.0078125; break;
		case 6: sweet = 0.015625; break;
		case 5: sweet = 0.03125; break;
		case 4: sweet = 0.0625; break;
		case 3: sweet = 0.125; break;
		case 2: sweet = 0.25; break;
		case 1: sweet = 0.5; break;
		case 0: sweet = 1.0; break;
		case -1: sweet = 2.0; break;
	} //now we have our input trim
		
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		
		double sweetSample = inputSample;
		
		double sv = sweetSample; sweetSample = (sweetSample + savg[0]) * 0.5; savg[0] = sv;
		if (cycleEnd > 1) {sv = sweetSample; sweetSample = (sweetSample + savg[1]) * 0.5; savg[1] = sv;
			if (cycleEnd > 2) {sv = sweetSample; sweetSample = (sweetSample + savg[2]) * 0.5; savg[2] = sv;
				if (cycleEnd > 3) {sv = sweetSample; sweetSample = (sweetSample + savg[3]) * 0.5; savg[3] = sv;}
			} //if undersampling code is present, this gives a simple averaging that stacks up
		} //when high sample rates are present, for a more intense aliasing reduction. PRE nonlinearity
		
		sweetSample = (sweetSample*sweetSample*sweet); //second harmonic (nonlinearity)
		
		sv = sweetSample; sweetSample = (sweetSample + savg[4]) * 0.5; savg[4] = sv;
		if (cycleEnd > 1) {sv = sweetSample; sweetSample = (sweetSample + savg[5]) * 0.5; savg[5] = sv;
			if (cycleEnd > 2) {sv = sweetSample; sweetSample = (sweetSample + savg[6]) * 0.5; savg[6] = sv;
				if (cycleEnd > 3) {sv = sweetSample; sweetSample = (sweetSample + savg[7]) * 0.5; savg[7] = sv;}
			} //if undersampling code is present, this gives a simple averaging that stacks up
		} //when high sample rates are present, for a more intense aliasing reduction. POST nonlinearity
		
		inputSample -= sweetSample; //apply the filtered second harmonic correction
		
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
	for (int x = 0; x < 8; x++) savg[x] = 0.0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

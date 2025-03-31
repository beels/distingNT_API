#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Slew2"
#define AIRWINDOWS_DESCRIPTION "Works like a de-esser or acceleration limiter: controls extreme highs."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','S','l','f' )
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
{ .name = "Clamping", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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
 
		Float32 ataLast3Sample;
		Float32 ataLast2Sample;
		Float32 ataLast1Sample;
		Float32 ataHalfwaySample;
		Float32 ataHalfDrySample;
		Float32 ataHalfDiffSample;
		Float32 ataA;
		Float32 ataB;
		Float32 ataC;
		Float32 ataDecay;
		Float32 ataUpsampleHighTweak;
		Float32 ataDrySample;
		Float32 ataDiffSample;
		Float32 ataPrevDiffSample;
		bool ataFlip; //end defining of antialiasing variables
		Float32 lastSample;
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
	Float32 overallscale = 2.0f;
	overallscale /= 44100.0f;
	overallscale *= GetSampleRate();
	Float32 inputSample;
	Float32 clamp;
	Float32 threshold = pow((1-GetParameter( kParam_One )),4)/overallscale;
	
	while (nSampleFrames-- > 0) {
		inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		ataDrySample = inputSample;
		
		ataHalfDrySample = ataHalfwaySample = (inputSample + ataLast1Sample + ((-ataLast2Sample + ataLast3Sample) * ataUpsampleHighTweak)) / 2.0f;
		ataLast3Sample = ataLast2Sample; ataLast2Sample = ataLast1Sample; ataLast1Sample = inputSample;
		//setting up oversampled special antialiasing
	//begin first half- change inputSample -> ataHalfwaySample, ataDrySample -> ataHalfDrySample
		clamp = ataHalfwaySample - ataHalfDrySample;
		if (clamp > threshold)
			ataHalfwaySample = lastSample + threshold;
		if (-clamp > threshold)
			ataHalfwaySample = lastSample - threshold;
		lastSample = ataHalfwaySample;
		

	//end first half
		//begin antialiasing section for halfway sample
		ataC = ataHalfwaySample - ataHalfDrySample;
		if (ataFlip) {ataA *= ataDecay; ataB *= ataDecay; ataA += ataC; ataB -= ataC; ataC = ataA;}
		else {ataB *= ataDecay; ataA *= ataDecay; ataB += ataC; ataA -= ataC; ataC = ataB;}
		ataHalfDiffSample = (ataC * ataDecay); ataFlip = !ataFlip;
		//end antialiasing section for halfway sample
	//begin second half- inputSample and ataDrySample handled separately here
		
		clamp = inputSample - lastSample;
		if (clamp > threshold)
			inputSample = lastSample + threshold;
		if (-clamp > threshold)
			inputSample = lastSample - threshold;
		lastSample = inputSample;
		

	//end second half
		//begin antialiasing section for input sample
		ataC = inputSample - ataDrySample;
		if (ataFlip) {ataA *= ataDecay; ataB *= ataDecay; ataA += ataC; ataB -= ataC; ataC = ataA;}
		else {ataB *= ataDecay; ataA *= ataDecay; ataB += ataC; ataA -= ataC; ataC = ataB;}
		ataDiffSample = (ataC * ataDecay); ataFlip = !ataFlip;
		//end antialiasing section for input sample
		inputSample = ataDrySample; inputSample += ((ataDiffSample + ataHalfDiffSample + ataPrevDiffSample) / 0.734f);
		ataPrevDiffSample = ataDiffSample / 2.0f;
		//apply processing as difference to non-oversampled raw input

		
		
		*destP = inputSample;
		//built in output trim and dry/wet by default
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	ataLast3Sample = ataLast2Sample = ataLast1Sample = 0.0;
	ataHalfwaySample = ataHalfDrySample = ataHalfDiffSample = 0.0;
	ataA = ataB = ataC = ataDrySample = ataDiffSample = ataPrevDiffSample = 0.0;
	ataUpsampleHighTweak = 0.0414213562373095048801688; //more adds treble to upsampling
	ataDecay = 0.915965594177219015; //Catalan's constant, more adds focus and clarity
	ataFlip = false; //end reset of antialias parameters
	lastSample = 0.0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

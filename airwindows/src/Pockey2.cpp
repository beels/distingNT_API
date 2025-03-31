#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Pockey2"
#define AIRWINDOWS_DESCRIPTION "More efficient, more intense lo-fi hiphop in a plugin."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','P','o','d' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	//Add your parameters here...
	kNumberOfParameters=3
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, kParam2, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "DeFreq", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "DeRez", .min = 4000, .max = 16000, .def = 12000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		double lastSample;
		double heldSample;
		double previousHeld;
		int position;
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
	double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();

	int freq = floor(pow(GetParameter( kParam_One ),3)*32.0*overallscale);
	//dividing of derez must always be integer values now: no freq grinding
	
	double rez = GetParameter( kParam_Two);
	//4 to 16, with 12 being the default.
	int rezFactor = (int)pow(2,rez); //256, 4096, 65536 or anything in between
	
	double wet = GetParameter( kParam_Three );
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		double drySample = inputSample;
				
		if (inputSample > 1.0) inputSample = 1.0; if (inputSample < -1.0) inputSample = -1.0;
		if (inputSample > 0) inputSample = log(1.0+(255*fabs(inputSample)))/log(255);
		if (inputSample < 0) inputSample = -log(1.0+(255*fabs(inputSample)))/log(255);
		//end uLaw encode		
		
		inputSample *= rezFactor;		
		if (inputSample > 0) inputSample = floor(inputSample);
		if (inputSample < 0) inputSample = -floor(-inputSample);
		inputSample /= rezFactor;
		
		if (inputSample > 1.0) inputSample = 1.0; if (inputSample < -1.0) inputSample = -1.0;
		if (inputSample > 0) inputSample = (pow(256,fabs(inputSample))-1.0) / 255;
		if (inputSample < 0) inputSample = -(pow(256,fabs(inputSample))-1.0) / 255;
		//end uLaw decode
		
		double blur = 0.618033988749894848204586-(fabs(inputSample - lastSample)*overallscale);
		if (blur < 0.0) blur = 0.0; //reverse it. Mellow stuff gets blur, bright gets edge
		
		if (position < 1)
		{
			position = freq; //one to ? scaled by overallscale
			heldSample = inputSample;			
		}
		inputSample = heldSample;
		lastSample = drySample;
		position--;

		inputSample = (inputSample * blur) + (previousHeld * (1.0-blur));
		//conditional average: only if we actually have brightness
		previousHeld = heldSample;
		//end Frequency Derez
		
		if (wet !=1.0) {
			inputSample = (inputSample * wet) + (drySample * (1.0-wet));
		}
		//Dry/Wet control, defaults to the last slider
		
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
	lastSample = 0.0;
	heldSample = 0.0;
	previousHeld = 0.0;
	position = 0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

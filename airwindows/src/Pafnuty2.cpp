#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Pafnuty2"
#define AIRWINDOWS_DESCRIPTION "A Chebyshev filter, that adds harmonics, and fits in the VCV Rack port."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','P','a','g' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	kParam_Five =4,
	kParam_Six =5,
	kParam_Seven =6,
	kParam_Eight =7,
	kParam_Nine =8,
	kParam_Ten =9,
	//Add your parameters here...
	kNumberOfParameters=10
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, kParam8, kParam9, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Second Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Third Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Fourth Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Fifth Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Sixth Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Seventh Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Eighth Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Ninth Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Tenth Harmonic", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Invert/Depth", .min = -1000, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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
	float chebyshev;
	float effect;
	float inP2;
	float inP3;
	float inP4;
	float inP5;
	float inP6;
	float inP7;
	float inP8;
	float inP9;
	float inP10;
	float second = (GetParameter( kParam_One )*1.0f);
	second = second * fabs(second);
	float third = -(GetParameter( kParam_Two )*0.60f);
	third = third * fabs(third);
	float fourth = -(GetParameter( kParam_Three )*0.60f);
	fourth = fourth * fabs(fourth);
	float fifth = (GetParameter( kParam_Four )*0.45f);
	fifth = fifth * fabs(fifth);
	float sixth = (GetParameter( kParam_Five )*0.45f);
	sixth = sixth * fabs(sixth);
	float seventh = -(GetParameter( kParam_Six )*0.38f);
	seventh = seventh * fabs(seventh);
	float eighth = -(GetParameter( kParam_Seven )*0.38f);
	eighth = eighth * fabs(eighth);
	float ninth = (GetParameter( kParam_Eight )*0.35f);
	ninth = ninth * fabs(ninth);
	float tenth = (GetParameter( kParam_Nine )*0.35f);
	tenth = tenth * fabs(tenth);
	float amount = GetParameter( kParam_Ten );
	amount = amount * fabs(amount);
	//setting up 
	
	while (nSampleFrames-- > 0) {
		float inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		
		effect = 0.0f;
		inP2 = inputSample * inputSample;
		inP3 = inP2 * inputSample;
		inP4 = inP3 * inputSample;
		inP5 = inP4 * inputSample;
		inP6 = inP5 * inputSample;
		inP7 = inP6 * inputSample;
		inP8 = inP7 * inputSample;
		inP9 = inP8 * inputSample;
		inP10 = inP9 * inputSample;
		//let's do the powers ahead of time and see how we do.
		if (second != 0.0f)
		{
			chebyshev = (2 * inP2);
			effect += (chebyshev * second);
		}
		if (third != 0.0f)
		{
			chebyshev = (4 * inP3) - (3*inputSample);
			effect += (chebyshev * third);
		}
		if (fourth != 0.0f)
		{
			chebyshev = (8 * inP4) - (8 * inP2);
			effect += (chebyshev * fourth);
		}
		if (fifth != 0.0f)
		{
			chebyshev = (16 * inP5) - (20 * inP3) + (5*inputSample);
			effect += (chebyshev * fifth);
		}
		if (sixth != 0.0f)
		{
			chebyshev = (32 * inP6) - (48 * inP4) + (18 * inP2);
			effect += (chebyshev * sixth);
		}
		if (seventh != 0.0f)
		{
			chebyshev = (64 * inP7) - (112 * inP5) + (56 * inP3) - (7*inputSample);
			effect += (chebyshev * seventh);
		}
		if (eighth != 0.0f)
		{
			chebyshev = (128 * inP8) - (256 * inP6) + (160 * inP4) - (32 * inP2);
			effect += (chebyshev * eighth);
		}
		if (ninth != 0.0f)
		{
			chebyshev = (256 * inP9) - (576 * inP7) + (432 * inP5) - (120 * inP3) + (9*inputSample);
			effect += (chebyshev * ninth);
		}
		if (tenth != 0.0f)
		{
			chebyshev = (512 * inP10) - (1280 * inP8) + (1120 * inP6) - (400 * inP4) + (50 * inP2);
			effect += (chebyshev * tenth);
		}
		//Yowza! Aren't we glad we're testing to see if we can skip these little bastards?
		inputSample += (effect * amount);
		//You too can make a horrible graunch and then SUBTRACT it leaving only the refreshing smell of pine...
		
		
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

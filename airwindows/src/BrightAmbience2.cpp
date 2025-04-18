#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "BrightAmbience2"
#define AIRWINDOWS_DESCRIPTION "More BrightAmbience with better tone and more slapbacky effects."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','B','r','j' )
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	//Add your parameters here...
	kNumberOfParameters=4
};
enum { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode,
kParamPrePostGain,
kParam0, kParam1, kParam2, kParam3, };
static const uint8_t page2[] = { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input L", 1, 1 )
NT_PARAMETER_AUDIO_INPUT( "Input R", 1, 2 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output L", 1, 13 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output R", 1, 14 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Start", .min = 0, .max = 1000, .def = 200, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Length", .min = 0, .max = 1000, .def = 200, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Feedback", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, };
enum { kNumTemplateParameters = 7 };
#include "../include/template1.h"
 
	int gcount;	
	float feedbackA;
	float feedbackB;
	float feedbackC;
	uint32_t fpdL;
	uint32_t fpdR;

	struct _dram {
		Float32 pL[32768];
	Float32 pR[32768];
	};
	_dram* dram;
#include "../include/template2.h"
#include "../include/templateStereo.h"
void _airwindowsAlgorithm::render( const Float32* inputL, const Float32* inputR, Float32* outputL, Float32* outputR, UInt32 inFramesToProcess ) {

	UInt32 nSampleFrames = inFramesToProcess;
	
	int start = (int)(pow(GetParameter( kParam_One ),2) * 480)+1;
	int length = (int)(pow(GetParameter( kParam_Two ),2) * 480)+1;
	if (start + length > 488) start = 488-length;
	Float32 feedbackAmount = 1.0f-(pow(1.0f-GetParameter( kParam_Three ),2));
	Float32 wet = GetParameter( kParam_Four );
	
	while (nSampleFrames-- > 0) {
		float inputSampleL = *inputL;
		float inputSampleR = *inputR;
		if (fabs(inputSampleL)<1.18e-23f) inputSampleL = fpdL * 1.18e-17f;
		if (fabs(inputSampleR)<1.18e-23f) inputSampleR = fpdR * 1.18e-17f;
		float drySampleL = inputSampleL;
		float drySampleR = inputSampleR;
		float tempL = 0.0f;
		float tempR = 0.0f;
		
		if (gcount < 0 || gcount > 32767) gcount = 32767;
		int count = gcount;
		
		dram->pL[count] = inputSampleL + ((sin(feedbackA)/sqrt(length+1))*feedbackAmount);
		dram->pR[count] = inputSampleR + ((sin(feedbackB)/sqrt(length+1))*feedbackAmount);

		for(int offset = start; offset < start+length; offset++) {
			tempL += dram->pL[count+primeL[offset] - ((count+primeL[offset] > 32767)?32768:0)];
			tempR += dram->pR[count+primeR[offset] - ((count+primeR[offset] > 32767)?32768:0)];
		}
		
		inputSampleL = tempL/sqrt(length);
		inputSampleR = tempR/sqrt(length);
		
		feedbackA = (feedbackA * (1.0f-feedbackAmount)) + (inputSampleR * feedbackAmount);
		feedbackB = (feedbackB * (1.0f-feedbackAmount)) + (inputSampleL * feedbackAmount);
		
		gcount--;
		
		if (wet != 1.0f) {
		 inputSampleL = (inputSampleL * wet) + (drySampleL * (1.0f-wet));
		 inputSampleR = (inputSampleR * wet) + (drySampleR * (1.0f-wet));
		}
		//Dry/Wet control, defaults to the last slider

		
		
		*outputL = inputSampleL;
		*outputR = inputSampleR;
		//direct stereo out
		
		inputL += 1;
		inputR += 1;
		outputL += 1;
		outputR += 1;
	}
	};
int _airwindowsAlgorithm::reset(void) {

{
	for(int count = 0; count < 32767; count++) {dram->pL[count] = 0.0; dram->pR[count] = 0.0;}
	feedbackA = feedbackB = feedbackC = 0.0;
	gcount = 0;
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	return noErr;
}

};

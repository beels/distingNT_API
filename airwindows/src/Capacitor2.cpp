#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Capacitor2"
#define AIRWINDOWS_DESCRIPTION "Capacitor with extra analog modeling and mojo."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','C','a','q' )
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
{ .name = "Lowpass", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Highpass", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "NonLin", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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
 
		
		Float64 iirHighpassA;
		Float64 iirHighpassB;
		Float64 iirHighpassC;
		Float64 iirHighpassD;
		Float64 iirHighpassE;
		Float64 iirHighpassF;
		Float64 iirLowpassA;
		Float64 iirLowpassB;
		Float64 iirLowpassC;
		Float64 iirLowpassD;
		Float64 iirLowpassE;
		Float64 iirLowpassF;
		int count;
		
		Float64 lowpassChase;
		Float64 highpassChase;
		Float64 wetChase;
		
		Float64 lowpassBaseAmount;
		Float64 highpassBaseAmount;
		Float64 wet;
		
		Float64 lastLowpass;
		Float64 lastHighpass;
		Float64 lastWet;
		
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
	
	lowpassChase = pow(GetParameter( kParam_One ),2);
	highpassChase = pow(GetParameter( kParam_Two ),2);
	Float64 nonLin = 1.0+((1.0-GetParameter( kParam_Three ))*6.0);
	Float64 nonLinTrim = 1.5/cbrt(nonLin);
	wetChase = GetParameter( kParam_Four );
	//should not scale with sample rate, because values reaching 1 are important
	//to its ability to bypass when set to max
	Float64 lowpassSpeed = 300 / (fabs( lastLowpass - lowpassChase)+1.0);
	Float64 highpassSpeed = 300 / (fabs( lastHighpass - highpassChase)+1.0);
	Float64 wetSpeed = 300 / (fabs( lastWet - wetChase)+1.0);
	lastLowpass = lowpassChase;
	lastHighpass = highpassChase;
	lastWet = wetChase;
		
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		double drySample = inputSample;
		
		Float64 dielectricScale = fabs(2.0-((inputSample+nonLin)/nonLin));
		lowpassBaseAmount = (((lowpassBaseAmount*lowpassSpeed)+lowpassChase)/(lowpassSpeed + 1.0));
		Float64 lowpassAmount = lowpassBaseAmount * dielectricScale;
		//positive voltage will mean lower capacitance when capacitor is barium titanate
		//on the lowpass, higher pressure means positive swings/smaller cap/larger value for lowpassAmount
		Float64 invLowpass = 1.0 - lowpassAmount;
		
		highpassBaseAmount = (((highpassBaseAmount*highpassSpeed)+highpassChase)/(highpassSpeed + 1.0));
		Float64 highpassAmount = highpassBaseAmount * dielectricScale;
		//positive voltage will mean lower capacitance when capacitor is barium titanate
		//on the highpass, higher pressure means positive swings/smaller cap/larger value for highpassAmount
		Float64 invHighpass = 1.0 - highpassAmount;

		wet = (((wet*wetSpeed)+wetChase)/(wetSpeed+1.0));
		
		count++; if (count > 5) count = 0; switch (count)
		{
			case 0:
				iirHighpassA = (iirHighpassA * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassA;
				iirLowpassA = (iirLowpassA * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassA;
				iirHighpassB = (iirHighpassB * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassB;
				iirLowpassB = (iirLowpassB * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassB;
				iirHighpassD = (iirHighpassD * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassD;
				iirLowpassD = (iirLowpassD * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassD;
				break;
			case 1:
				iirHighpassA = (iirHighpassA * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassA;
				iirLowpassA = (iirLowpassA * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassA;
				iirHighpassC = (iirHighpassC * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassC;
				iirLowpassC = (iirLowpassC * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassC;
				iirHighpassE = (iirHighpassE * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassE;
				iirLowpassE = (iirLowpassE * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassE;
				break;
			case 2:
				iirHighpassA = (iirHighpassA * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassA;
				iirLowpassA = (iirLowpassA * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassA;
				iirHighpassB = (iirHighpassB * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassB;
				iirLowpassB = (iirLowpassB * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassB;
				iirHighpassF = (iirHighpassF * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassF;
				iirLowpassF = (iirLowpassF * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassF;
				break;
			case 3:
				iirHighpassA = (iirHighpassA * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassA;
				iirLowpassA = (iirLowpassA * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassA;
				iirHighpassC = (iirHighpassC * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassC;
				iirLowpassC = (iirLowpassC * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassC;
				iirHighpassD = (iirHighpassD * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassD;
				iirLowpassD = (iirLowpassD * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassD;
				break;
			case 4:
				iirHighpassA = (iirHighpassA * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassA;
				iirLowpassA = (iirLowpassA * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassA;
				iirHighpassB = (iirHighpassB * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassB;
				iirLowpassB = (iirLowpassB * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassB;
				iirHighpassE = (iirHighpassE * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassE;
				iirLowpassE = (iirLowpassE * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassE;
				break;
			case 5:
				iirHighpassA = (iirHighpassA * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassA;
				iirLowpassA = (iirLowpassA * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassA;
				iirHighpassC = (iirHighpassC * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassC;
				iirLowpassC = (iirLowpassC * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassC;
				iirHighpassF = (iirHighpassF * invHighpass) + (inputSample * highpassAmount); inputSample -= iirHighpassF;
				iirLowpassF = (iirLowpassF * invLowpass) + (inputSample * lowpassAmount); inputSample = iirLowpassF;
				break;
		}
		//Highpass Filter chunk. This is three poles of IIR highpass, with a 'gearbox' that progressively
		//steepens the filter after minimizing artifacts.
		
		inputSample = (drySample * (1.0-wet)) + (inputSample * nonLinTrim * wet);
		
		//begin 32 bit floating point dither
		int expon; frexpf((float)inputSample, &expon);
		fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
		inputSample += static_cast<int32_t>(fpd) * 5.960464655174751e-36L * pow(2,expon+62);
		//end 32 bit floating point dither
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	iirHighpassA = 0.0;
	iirHighpassB = 0.0;
	iirHighpassC = 0.0;
	iirHighpassD = 0.0;
	iirHighpassE = 0.0;
	iirHighpassF = 0.0;
	iirLowpassA = 0.0;
	iirLowpassB = 0.0;
	iirLowpassC = 0.0;
	iirLowpassD = 0.0;
	iirLowpassE = 0.0;
	iirLowpassF = 0.0;
	count = 0;
	lowpassChase = 0.0;
	highpassChase = 0.0;
	wetChase = 0.0;
	lowpassBaseAmount = 1.0;
	highpassBaseAmount = 0.0;
	wet = 1.0;
	lastLowpass = 1000.0;
	lastHighpass = 1000.0;
	lastWet = 1000.0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

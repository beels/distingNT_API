#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Discontinuity"
#define AIRWINDOWS_DESCRIPTION "Models air under intense loudness."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','D','i','s' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	//Add your parameters here...
	kNumberOfParameters=1
};
const int dscBuf = 90;
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Top dB", .min = 7000, .max = 14000, .def = 10000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL },
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
 
		
		double dBaL[dscBuf+5];
		double dBaPosL;
		int dBaXL;		
		
		double dBbL[dscBuf+5];
		double dBbPosL;
		int dBbXL;		
		
		double dBcL[dscBuf+5];
		double dBcPosL;
		int dBcXL;		
				
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
	
	double refdB = GetParameter( kParam_One );
	double topdB = 0.000000075 * pow(10.0,refdB/20.0) * overallscale;
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		
		inputSample *= topdB;
		if (inputSample < -0.222) inputSample = -0.222; if (inputSample > 0.222) inputSample = 0.222;
		//Air Discontinuity A begin
		dBaL[dBaXL] = inputSample; dBaPosL *= 0.5; dBaPosL += fabs((inputSample*((inputSample*0.25)-0.5))*0.5);
		int dBdly = floor(dBaPosL*dscBuf);
		double dBi = (dBaPosL*dscBuf)-dBdly;
		inputSample = dBaL[dBaXL-dBdly +((dBaXL-dBdly < 0)?dscBuf:0)]*(1.0-dBi);
		dBdly++; inputSample += dBaL[dBaXL-dBdly +((dBaXL-dBdly < 0)?dscBuf:0)]*dBi;
		dBaXL++; if (dBaXL < 0 || dBaXL >= dscBuf) dBaXL = 0;
		//Air Discontinuity A end
		//Air Discontinuity B begin
		dBbL[dBbXL] = inputSample;  dBbPosL *= 0.5; dBbPosL += fabs((inputSample*((inputSample*0.25)-0.5))*0.5);
		dBdly = floor(dBbPosL*dscBuf); dBi = (dBbPosL*dscBuf)-dBdly;
		inputSample = dBbL[dBbXL-dBdly +((dBbXL-dBdly < 0)?dscBuf:0)]*(1.0-dBi);
		dBdly++; inputSample += dBbL[dBbXL-dBdly +((dBbXL-dBdly < 0)?dscBuf:0)]*dBi;
		dBbXL++; if (dBbXL < 0 || dBbXL >= dscBuf) dBbXL = 0;
		//Air Discontinuity B end
		//Air Discontinuity C begin
		dBcL[dBcXL] = inputSample;  dBcPosL *= 0.5; dBcPosL += fabs((inputSample*((inputSample*0.25)-0.5))*0.5);
		dBdly = floor(dBcPosL*dscBuf); dBi = (dBcPosL*dscBuf)-dBdly;
		inputSample = dBcL[dBcXL-dBdly +((dBcXL-dBdly < 0)?dscBuf:0)]*(1.0-dBi);
		dBdly++; inputSample += dBcL[dBcXL-dBdly +((dBcXL-dBdly < 0)?dscBuf:0)]*dBi;
		dBcXL++; if (dBcXL < 0 || dBcXL >= dscBuf) dBcXL = 0;
		//Air Discontinuity C end
		inputSample /= topdB;
		
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
	for(int count = 0; count < dscBuf+2; count++) {
		dBaL[count] = 0.0;
		dBbL[count] = 0.0;
		dBcL[count] = 0.0;
	}
	dBaPosL = 0.0;
	dBbPosL = 0.0;
	dBcPosL = 0.0;
	dBaXL = 1;
	dBbXL = 1;
	dBcXL = 1;
	
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Mastering"
#define AIRWINDOWS_DESCRIPTION "Airwindows style, and can do things nothing else can!"
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','M','a','s' )
enum {

	kParam_A =0,
	kParam_B =1,
	kParam_C =2,
	kParam_D =3,
	kParam_E =4,
	kParam_F =5,
	//Add your parameters here...
	kNumberOfParameters=6
};
static const int kDark = 1;
static const int kTenNines = 2;
static const int kTPDFWide = 3;
static const int kPaulWide = 4;
static const int kNJAD = 5;
static const int kBypass = 6;
static const int kDefaultValue_ParamF = kBypass;
enum { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode,
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, };
static char const * const enumStrings5[] = { "", "Dark", "Ten Nines", "TPDFWide", "PaulWide", "NJAD", "Bypass", };
static const uint8_t page2[] = { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input L", 1, 1 )
NT_PARAMETER_AUDIO_INPUT( "Input R", 1, 2 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output L", 1, 13 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output R", 1, 14 )
{ .name = "Glue", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Scope", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Skronk", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Girth", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Drive", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dither", .min = 1, .max = 6, .def = 6, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = enumStrings5 },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, };
enum { kNumTemplateParameters = 6 };
#include "../include/template1.h"

	
	enum {
		pvAL1,
		pvSL1,
		accSL1,
		acc2SL1,
		pvAL2,
		pvSL2,
		accSL2,
		acc2SL2,
		pvAL3,
		pvSL3,
		accSL3,
		pvAL4,
		pvSL4,
		gndavgL,
		outAL,
		gainAL,
		pvAR1,
		pvSR1,
		accSR1,
		acc2SR1,
		pvAR2,
		pvSR2,
		accSR2,
		acc2SR2,
		pvAR3,
		pvSR3,
		accSR3,
		pvAR4,
		pvSR4,
		gndavgR,
		outAR,
		gainAR,
		air_total
	};
	
	enum {
		prevSampL1,
		prevSlewL1,
		accSlewL1,
		prevSampL2,
		prevSlewL2,
		accSlewL2,
		prevSampL3,
		prevSlewL3,
		accSlewL3,
		kalGainL,
		kalOutL,
		kalAvgL,
		prevSampR1,
		prevSlewR1,
		accSlewR1,
		prevSampR2,
		prevSlewR2,
		accSlewR2,
		prevSampR3,
		prevSlewR3,
		accSlewR3,
		kalGainR,
		kalOutR,
		kalAvgR,
		kal_total
	};
	
	double lastSinewL;
	double lastSinewR;
	//this is overkill, used to run both Zoom and Sinew stages as they are after
	//the summing in StoneFire, which sums three doubles to a long double.
	
	double lastSampleL;
	double intermediateL[16];
	bool wasPosClipL;
	bool wasNegClipL;
	double lastSampleR;
	double intermediateR[16];
	bool wasPosClipR;
	bool wasNegClipR; //Stereo ClipOnly2
	
	int quantA;
	int quantB;
	float expectedSlew;
	float testA;
	float testB;
	double correction;
	double shapedSampleL;
	double shapedSampleR;
	double currentDither;
	double ditherL;
	double ditherR;
	bool cutbinsL;
	bool cutbinsR;
	int hotbinA;
	int hotbinB;
	double benfordize;
	double totalA;
	double totalB;
	double outputSample;
	int expon; //internal dither variables

	double NSOddL; //dither section!
	double NSEvenL;
	double prevShapeL;
	double NSOddR;
	double NSEvenR;
	double prevShapeR;
	bool flip; //VinylDither
	double previousDitherL;
	double previousDitherR; //PaulWide
	double bynL[13], bynR[13];
	double noiseShapingL, noiseShapingR; //NJAD
		
	uint32_t fpdL;
	uint32_t fpdR;

	struct _dram {
		double air[air_total];
	double kalM[kal_total];
	double kalS[kal_total];
	double darkSampleL[100];
	double darkSampleR[100]; //Dark
	};
	_dram* dram;
#include "../include/template2.h"
#include "../include/templateStereo.h"
void _airwindowsAlgorithm::render( const Float32* inputL, const Float32* inputR, Float32* outputL, Float32* outputR, UInt32 inFramesToProcess ) {

	UInt32 nSampleFrames = inFramesToProcess;
	double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	
	double threshSinew = (0.25+((1.0-GetParameter( kParam_A ))*0.333))/overallscale;
	double depthSinew = 1.0-pow(1.0-GetParameter( kParam_A ),2.0);
	
	double trebleZoom = GetParameter( kParam_B )-0.5;
	double trebleGain = (trebleZoom*fabs(trebleZoom))+1.0;
	if (trebleGain > 1.0) trebleGain = pow(trebleGain,3.0+sqrt(overallscale));
	//this boost is necessary to adapt to higher sample rates
	
	double midZoom = GetParameter( kParam_C )-0.5;
	double midGain = (midZoom*fabs(midZoom))+1.0;
	double kalMid = 0.35-(GetParameter( kParam_C )*0.25); //crossover frequency between mid/bass
	double kalSub = 0.45+(GetParameter( kParam_C )*0.25); //crossover frequency between bass/sub
	
	double bassZoom = (GetParameter( kParam_D )*0.5)-0.25;
	double bassGain = (-bassZoom*fabs(bassZoom))+1.0; //control inverted
	double subGain = ((GetParameter( kParam_D )*0.25)-0.125)+1.0;
	if (subGain < 1.0) subGain = 1.0; //very small sub shift, only pos.
	
	double driveIn = (GetParameter( kParam_E )-0.5)+1.0;
	double driveOut = (-(GetParameter( kParam_E )-0.5)*fabs(GetParameter( kParam_E )-0.5))+1.0;
	
	int spacing = floor(overallscale); //should give us working basic scaling, usually 2 or 4
	if (spacing < 1) spacing = 1; if (spacing > 16) spacing = 16;
	int dither = (int) GetParameter( kParam_F );
	int depth = (int)(17.0*overallscale);
	if (depth < 3) depth = 3; if (depth > 98) depth = 98; //for Dark
	
	while (nSampleFrames-- > 0) {
		double inputSampleL = *inputL;
		double inputSampleR = *inputR;
		if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;		
		inputSampleL *= driveIn;
		inputSampleR *= driveIn;
		double drySampleL = inputSampleL;
		double drySampleR = inputSampleR;
		
		//begin Air3L
		dram->air[pvSL4] = dram->air[pvAL4] - dram->air[pvAL3]; dram->air[pvSL3] = dram->air[pvAL3] - dram->air[pvAL2];
		dram->air[pvSL2] = dram->air[pvAL2] - dram->air[pvAL1]; dram->air[pvSL1] = dram->air[pvAL1] - inputSampleL;
		dram->air[accSL3] = dram->air[pvSL4] - dram->air[pvSL3]; dram->air[accSL2] = dram->air[pvSL3] - dram->air[pvSL2];
		dram->air[accSL1] = dram->air[pvSL2] - dram->air[pvSL1];
		dram->air[acc2SL2] = dram->air[accSL3] - dram->air[accSL2]; dram->air[acc2SL1] = dram->air[accSL2] - dram->air[accSL1];		
		dram->air[outAL] = -(dram->air[pvAL1] + dram->air[pvSL3] + dram->air[acc2SL2] - ((dram->air[acc2SL2] + dram->air[acc2SL1])*0.5));
		dram->air[gainAL] *= 0.5; dram->air[gainAL] += fabs(drySampleL-dram->air[outAL])*0.5;
		if (dram->air[gainAL] > 0.3*sqrt(overallscale)) dram->air[gainAL] = 0.3*sqrt(overallscale);
		dram->air[pvAL4] = dram->air[pvAL3]; dram->air[pvAL3] = dram->air[pvAL2];
		dram->air[pvAL2] = dram->air[pvAL1]; dram->air[pvAL1] = (dram->air[gainAL] * dram->air[outAL]) + drySampleL;
		double midL = drySampleL - ((dram->air[outAL]*0.5)+(drySampleL*(0.457-(0.017*overallscale))));
		double temp = (midL + dram->air[gndavgL])*0.5; dram->air[gndavgL] = midL; midL = temp;
		double trebleL = drySampleL-midL;
		//end Air3L
		
		//begin Air3R
		dram->air[pvSR4] = dram->air[pvAR4] - dram->air[pvAR3]; dram->air[pvSR3] = dram->air[pvAR3] - dram->air[pvAR2];
		dram->air[pvSR2] = dram->air[pvAR2] - dram->air[pvAR1]; dram->air[pvSR1] = dram->air[pvAR1] - inputSampleR;
		dram->air[accSR3] = dram->air[pvSR4] - dram->air[pvSR3]; dram->air[accSR2] = dram->air[pvSR3] - dram->air[pvSR2];
		dram->air[accSR1] = dram->air[pvSR2] - dram->air[pvSR1];
		dram->air[acc2SR2] = dram->air[accSR3] - dram->air[accSR2]; dram->air[acc2SR1] = dram->air[accSR2] - dram->air[accSR1];		
		dram->air[outAR] = -(dram->air[pvAR1] + dram->air[pvSR3] + dram->air[acc2SR2] - ((dram->air[acc2SR2] + dram->air[acc2SR1])*0.5));
		dram->air[gainAR] *= 0.5; dram->air[gainAR] += fabs(drySampleR-dram->air[outAR])*0.5;
		if (dram->air[gainAR] > 0.3*sqrt(overallscale)) dram->air[gainAR] = 0.3*sqrt(overallscale);
		dram->air[pvAR4] = dram->air[pvAR3]; dram->air[pvAR3] = dram->air[pvAR2];
		dram->air[pvAR2] = dram->air[pvAR1]; dram->air[pvAR1] = (dram->air[gainAR] * dram->air[outAR]) + drySampleR;
		double midR = drySampleR - ((dram->air[outAR]*0.5)+(drySampleR*(0.457-(0.017*overallscale))));
		temp = (midR + dram->air[gndavgR])*0.5; dram->air[gndavgR] = midR; midR = temp;
		double trebleR = drySampleR-midR;
		//end Air3R
		
		//begin KalmanML
		temp = midL;
		dram->kalM[prevSlewL3] += dram->kalM[prevSampL3] - dram->kalM[prevSampL2]; dram->kalM[prevSlewL3] *= 0.5;
		dram->kalM[prevSlewL2] += dram->kalM[prevSampL2] - dram->kalM[prevSampL1]; dram->kalM[prevSlewL2] *= 0.5;
		dram->kalM[prevSlewL1] += dram->kalM[prevSampL1] - midL; dram->kalM[prevSlewL1] *= 0.5;
		//make slews from each set of samples used
		dram->kalM[accSlewL2] += dram->kalM[prevSlewL3] - dram->kalM[prevSlewL2]; dram->kalM[accSlewL2] *= 0.5;
		dram->kalM[accSlewL1] += dram->kalM[prevSlewL2] - dram->kalM[prevSlewL1]; dram->kalM[accSlewL1] *= 0.5;
		//differences between slews: rate of change of rate of change
		dram->kalM[accSlewL3] += (dram->kalM[accSlewL2] - dram->kalM[accSlewL1]); dram->kalM[accSlewL3] *= 0.5;
		//entering the abyss, what even is this
		dram->kalM[kalOutL] += dram->kalM[prevSampL1] + dram->kalM[prevSlewL2] + dram->kalM[accSlewL3]; dram->kalM[kalOutL] *= 0.5;
		
		//resynthesizing predicted result (all iir smoothed)
		dram->kalM[kalGainL] += fabs(temp-dram->kalM[kalOutL])*kalMid*8.0; dram->kalM[kalGainL] *= 0.5;
		//madness takes its toll. Kalman Gain: how much dry to retain
		if (dram->kalM[kalGainL] > kalMid*0.5) dram->kalM[kalGainL] = kalMid*0.5;
		//attempts to avoid explosions
		dram->kalM[kalOutL] += (temp*(1.0-(0.68+(kalMid*0.157))));	
		//this is for tuning a really complete cancellation up around Nyquist
		dram->kalM[prevSampL3] = dram->kalM[prevSampL2]; dram->kalM[prevSampL2] = dram->kalM[prevSampL1];
		dram->kalM[prevSampL1] = (dram->kalM[kalGainL] * dram->kalM[kalOutL]) + ((1.0-dram->kalM[kalGainL])*temp);
		//feed the chain of previous samples
		double bassL = (dram->kalM[kalOutL]+dram->kalM[kalAvgL])*0.5;
		dram->kalM[kalAvgL] = dram->kalM[kalOutL];
		midL -= bassL;
		//end KalmanML
		
		//begin KalmanMR
		temp = midR;
		dram->kalM[prevSlewR3] += dram->kalM[prevSampR3] - dram->kalM[prevSampR2]; dram->kalM[prevSlewR3] *= 0.5;
		dram->kalM[prevSlewR2] += dram->kalM[prevSampR2] - dram->kalM[prevSampR1]; dram->kalM[prevSlewR2] *= 0.5;
		dram->kalM[prevSlewR1] += dram->kalM[prevSampR1] - midR; dram->kalM[prevSlewR1] *= 0.5;
		//make slews from each set of samples used
		dram->kalM[accSlewR2] += dram->kalM[prevSlewR3] - dram->kalM[prevSlewR2]; dram->kalM[accSlewR2] *= 0.5;
		dram->kalM[accSlewR1] += dram->kalM[prevSlewR2] - dram->kalM[prevSlewR1]; dram->kalM[accSlewR1] *= 0.5;
		//differences between slews: rate of change of rate of change
		dram->kalM[accSlewR3] += (dram->kalM[accSlewR2] - dram->kalM[accSlewR1]); dram->kalM[accSlewR3] *= 0.5;
		//entering the abyss, what even is this
		dram->kalM[kalOutR] += dram->kalM[prevSampR1] + dram->kalM[prevSlewR2] + dram->kalM[accSlewR3]; dram->kalM[kalOutR] *= 0.5;
		
		//resynthesizing predicted result (all iir smoothed)
		dram->kalM[kalGainR] += fabs(temp-dram->kalM[kalOutR])*kalMid*8.0; dram->kalM[kalGainR] *= 0.5;
		//madness takes its toll. Kalman Gain: how much dry to retain
		if (dram->kalM[kalGainR] > kalMid*0.5) dram->kalM[kalGainR] = kalMid*0.5;
		//attempts to avoid explosions
		dram->kalM[kalOutR] += (temp*(1.0-(0.68+(kalMid*0.157))));	
		//this is for tuning a really complete cancellation up around Nyquist
		dram->kalM[prevSampR3] = dram->kalM[prevSampR2]; dram->kalM[prevSampR2] = dram->kalM[prevSampR1];
		dram->kalM[prevSampR1] = (dram->kalM[kalGainR] * dram->kalM[kalOutR]) + ((1.0-dram->kalM[kalGainR])*temp);
		//feed the chain of previous samples
		double bassR = (dram->kalM[kalOutR]+dram->kalM[kalAvgR])*0.5;
		dram->kalM[kalAvgR] = dram->kalM[kalOutR];
		midR -= bassR;
		//end KalmanMR
		
		//begin KalmanSL
		temp = bassL;
		dram->kalS[prevSlewL3] += dram->kalS[prevSampL3] - dram->kalS[prevSampL2]; dram->kalS[prevSlewL3] *= 0.5;
		dram->kalS[prevSlewL2] += dram->kalS[prevSampL2] - dram->kalS[prevSampL1]; dram->kalS[prevSlewL2] *= 0.5;
		dram->kalS[prevSlewL1] += dram->kalS[prevSampL1] - bassL; dram->kalS[prevSlewL1] *= 0.5;
		//make slews from each set of samples used
		dram->kalS[accSlewL2] += dram->kalS[prevSlewL3] - dram->kalS[prevSlewL2]; dram->kalS[accSlewL2] *= 0.5;
		dram->kalS[accSlewL1] += dram->kalS[prevSlewL2] - dram->kalS[prevSlewL1]; dram->kalS[accSlewL1] *= 0.5;
		//differences between slews: rate of change of rate of change
		dram->kalS[accSlewL3] += (dram->kalS[accSlewL2] - dram->kalS[accSlewL1]); dram->kalS[accSlewL3] *= 0.5;
		//entering the abyss, what even is this
		dram->kalS[kalOutL] += dram->kalS[prevSampL1] + dram->kalS[prevSlewL2] + dram->kalS[accSlewL3]; dram->kalS[kalOutL] *= 0.5;
		//resynthesizing predicted result (all iir smoothed)
		dram->kalS[kalGainL] += fabs(temp-dram->kalS[kalOutL])*kalSub*8.0; dram->kalS[kalGainL] *= 0.5;
		//madness takes its toll. Kalman Gain: how much dry to retain
		if (dram->kalS[kalGainL] > kalSub*0.5) dram->kalS[kalGainL] = kalSub*0.5;
		//attempts to avoid explosions
		dram->kalS[kalOutL] += (temp*(1.0-(0.68+(kalSub*0.157))));	
		//this is for tuning a really complete cancellation up around Nyquist
		dram->kalS[prevSampL3] = dram->kalS[prevSampL2]; dram->kalS[prevSampL2] = dram->kalS[prevSampL1];
		dram->kalS[prevSampL1] = (dram->kalS[kalGainL] * dram->kalS[kalOutL]) + ((1.0-dram->kalS[kalGainL])*temp);
		//feed the chain of previous samples
		double subL = (dram->kalS[kalOutL]+dram->kalS[kalAvgL])*0.5;
		dram->kalS[kalAvgL] = dram->kalS[kalOutL];
		bassL -= subL;
		//end KalmanSL
		
		//begin KalmanSR
		temp = bassR;
		dram->kalS[prevSlewR3] += dram->kalS[prevSampR3] - dram->kalS[prevSampR2]; dram->kalS[prevSlewR3] *= 0.5;
		dram->kalS[prevSlewR2] += dram->kalS[prevSampR2] - dram->kalS[prevSampR1]; dram->kalS[prevSlewR2] *= 0.5;
		dram->kalS[prevSlewR1] += dram->kalS[prevSampR1] - bassR; dram->kalS[prevSlewR1] *= 0.5;
		//make slews from each set of samples used
		dram->kalS[accSlewR2] += dram->kalS[prevSlewR3] - dram->kalS[prevSlewR2]; dram->kalS[accSlewR2] *= 0.5;
		dram->kalS[accSlewR1] += dram->kalS[prevSlewR2] - dram->kalS[prevSlewR1]; dram->kalS[accSlewR1] *= 0.5;
		//differences between slews: rate of change of rate of change
		dram->kalS[accSlewR3] += (dram->kalS[accSlewR2] - dram->kalS[accSlewR1]); dram->kalS[accSlewR3] *= 0.5;
		//entering the abyss, what even is this
		dram->kalS[kalOutR] += dram->kalS[prevSampR1] + dram->kalS[prevSlewR2] + dram->kalS[accSlewR3]; dram->kalS[kalOutR] *= 0.5;
		//resynthesizing predicted result (all iir smoothed)
		dram->kalS[kalGainR] += fabs(temp-dram->kalS[kalOutR])*kalSub*8.0; dram->kalS[kalGainR] *= 0.5;
		//madness takes its toll. Kalman Gain: how much dry to retain
		if (dram->kalS[kalGainR] > kalSub*0.5) dram->kalS[kalGainR] = kalSub*0.5;
		//attempts to avoid explosions
		dram->kalS[kalOutR] += (temp*(1.0-(0.68+(kalSub*0.157))));	
		//this is for tuning a really complete cancellation up around Nyquist
		dram->kalS[prevSampR3] = dram->kalS[prevSampR2]; dram->kalS[prevSampR2] = dram->kalS[prevSampR1];
		dram->kalS[prevSampR1] = (dram->kalS[kalGainR] * dram->kalS[kalOutR]) + ((1.0-dram->kalS[kalGainR])*temp);
		//feed the chain of previous samples
		double subR = (dram->kalS[kalOutR]+dram->kalS[kalAvgR])*0.5;
		dram->kalS[kalAvgR] = dram->kalS[kalOutR];
		bassR -= subR;
		//end KalmanSR
		inputSampleL = (subL*subGain);
		inputSampleR = (subR*subGain);
		
		if (bassZoom > 0.0) {
			double closer = bassL * 1.57079633;
			if (closer > 1.57079633) closer = 1.57079633;
			if (closer < -1.57079633) closer = -1.57079633;
			bassL = (bassL*(1.0-bassZoom))+(sin(closer)*bassZoom);
			closer = bassR * 1.57079633;
			if (closer > 1.57079633) closer = 1.57079633;
			if (closer < -1.57079633) closer = -1.57079633;
			bassR = (bassR*(1.0-bassZoom))+(sin(closer)*bassZoom);
		} //zooming in will make the body of the sound louder: it's just Density
		if (bassZoom < 0.0) {
			double farther = fabs(bassL) * 1.57079633;
			if (farther > 1.57079633) farther = 1.0;
			else farther = 1.0-cos(farther);
			if (bassL > 0.0) bassL = (bassL*(1.0+bassZoom))-(farther*bassZoom*1.57079633);
			if (bassL < 0.0) bassL = (bassL*(1.0+bassZoom))+(farther*bassZoom*1.57079633);			
			farther = fabs(bassR) * 1.57079633;
			if (farther > 1.57079633) farther = 1.0;
			else farther = 1.0-cos(farther);
			if (bassR > 0.0) bassR = (bassR*(1.0+bassZoom))-(farther*bassZoom*1.57079633);
			if (bassR < 0.0) bassR = (bassR*(1.0+bassZoom))+(farther*bassZoom*1.57079633);			
		} //zooming out boosts the hottest peaks but cuts back softer stuff
		inputSampleL += (bassL*bassGain);
		inputSampleR += (bassR*bassGain);
		
		if (midZoom > 0.0) {
			double closer = midL * 1.57079633;
			if (closer > 1.57079633) closer = 1.57079633;
			if (closer < -1.57079633) closer = -1.57079633;
			midL = (midL*(1.0-midZoom))+(sin(closer)*midZoom);
			closer = midR * 1.57079633;
			if (closer > 1.57079633) closer = 1.57079633;
			if (closer < -1.57079633) closer = -1.57079633;
			midR = (midR*(1.0-midZoom))+(sin(closer)*midZoom);
		} //zooming in will make the body of the sound louder: it's just Density
		if (midZoom < 0.0) {
			double farther = fabs(midL) * 1.57079633;
			if (farther > 1.57079633) farther = 1.0;
			else farther = 1.0-cos(farther);
			if (midL > 0.0) midL = (midL*(1.0+midZoom))-(farther*midZoom*1.57079633);
			if (midL < 0.0) midL = (midL*(1.0+midZoom))+(farther*midZoom*1.57079633);			
			farther = fabs(midR) * 1.57079633;
			if (farther > 1.57079633) farther = 1.0;
			else farther = 1.0-cos(farther);
			if (midR > 0.0) midR = (midR*(1.0+midZoom))-(farther*midZoom*1.57079633);
			if (midR < 0.0) midR = (midR*(1.0+midZoom))+(farther*midZoom*1.57079633);			
		} //zooming out boosts the hottest peaks but cuts back softer stuff
		inputSampleL += (midL*midGain);
		inputSampleR += (midR*midGain);
		
		if (trebleZoom > 0.0) {
			double closer = trebleL * 1.57079633;
			if (closer > 1.57079633) closer = 1.57079633;
			if (closer < -1.57079633) closer = -1.57079633;
			trebleL = (trebleL*(1.0-trebleZoom))+(sin(closer)*trebleZoom);
			closer = trebleR * 1.57079633;
			if (closer > 1.57079633) closer = 1.57079633;
			if (closer < -1.57079633) closer = -1.57079633;
			trebleR = (trebleR*(1.0-trebleZoom))+(sin(closer)*trebleZoom);
		} //zooming in will make the body of the sound louder: it's just Density
		if (trebleZoom < 0.0) {
			double farther = fabs(trebleL) * 1.57079633;
			if (farther > 1.57079633) farther = 1.0;
			else farther = 1.0-cos(farther);
			if (trebleL > 0.0) trebleL = (trebleL*(1.0+trebleZoom))-(farther*trebleZoom*1.57079633);
			if (trebleL < 0.0) trebleL = (trebleL*(1.0+trebleZoom))+(farther*trebleZoom*1.57079633);			
			farther = fabs(trebleR) * 1.57079633;
			if (farther > 1.57079633) farther = 1.0;
			else farther = 1.0-cos(farther);
			if (trebleR > 0.0) trebleR = (trebleR*(1.0+trebleZoom))-(farther*trebleZoom*1.57079633);
			if (trebleR < 0.0) trebleR = (trebleR*(1.0+trebleZoom))+(farther*trebleZoom*1.57079633);			
		} //zooming out boosts the hottest peaks but cuts back softer stuff
		inputSampleL += (trebleL*trebleGain);
		inputSampleR += (trebleR*trebleGain);
		
		inputSampleL *= driveOut;
		inputSampleR *= driveOut;
				
		//begin ClipOnly2 stereo as a little, compressed chunk that can be dropped into code
		if (inputSampleL > 4.0) inputSampleL = 4.0; if (inputSampleL < -4.0) inputSampleL = -4.0;
		if (wasPosClipL == true) { //current will be over
			if (inputSampleL<lastSampleL) lastSampleL=0.7058208+(inputSampleL*0.2609148);
			else lastSampleL = 0.2491717+(lastSampleL*0.7390851);
		} wasPosClipL = false;
		if (inputSampleL>0.9549925859) {wasPosClipL=true;inputSampleL=0.7058208+(lastSampleL*0.2609148);}
		if (wasNegClipL == true) { //current will be -over
			if (inputSampleL > lastSampleL) lastSampleL=-0.7058208+(inputSampleL*0.2609148);
			else lastSampleL=-0.2491717+(lastSampleL*0.7390851);
		} wasNegClipL = false;
		if (inputSampleL<-0.9549925859) {wasNegClipL=true;inputSampleL=-0.7058208+(lastSampleL*0.2609148);}
		intermediateL[spacing] = inputSampleL;
        inputSampleL = lastSampleL; //Latency is however many samples equals one 44.1k sample
		for (int x = spacing; x > 0; x--) intermediateL[x-1] = intermediateL[x];
		lastSampleL = intermediateL[0]; //run a little buffer to handle this
		
		if (inputSampleR > 4.0) inputSampleR = 4.0; if (inputSampleR < -4.0) inputSampleR = -4.0;
		if (wasPosClipR == true) { //current will be over
			if (inputSampleR<lastSampleR) lastSampleR=0.7058208+(inputSampleR*0.2609148);
			else lastSampleR = 0.2491717+(lastSampleR*0.7390851);
		} wasPosClipR = false;
		if (inputSampleR>0.9549925859) {wasPosClipR=true;inputSampleR=0.7058208+(lastSampleR*0.2609148);}
		if (wasNegClipR == true) { //current will be -over
			if (inputSampleR > lastSampleR) lastSampleR=-0.7058208+(inputSampleR*0.2609148);
			else lastSampleR=-0.2491717+(lastSampleR*0.7390851);
		} wasNegClipR = false;
		if (inputSampleR<-0.9549925859) {wasNegClipR=true;inputSampleR=-0.7058208+(lastSampleR*0.2609148);}
		intermediateR[spacing] = inputSampleR;
        inputSampleR = lastSampleR; //Latency is however many samples equals one 44.1k sample
		for (int x = spacing; x > 0; x--) intermediateR[x-1] = intermediateR[x];
		lastSampleR = intermediateR[0]; //run a little buffer to handle this
		//end ClipOnly2 stereo as a little, compressed chunk that can be dropped into code
		
		temp = inputSampleL;
		double sinew = threshSinew * cos(lastSinewL*lastSinewL);
		if (inputSampleL - lastSinewL > sinew) temp = lastSinewL + sinew;
		if (-(inputSampleL - lastSinewL) > sinew) temp = lastSinewL - sinew;
		lastSinewL = temp;
		inputSampleL = (inputSampleL * (1.0-depthSinew))+(lastSinewL*depthSinew);
		temp = inputSampleR;
		sinew = threshSinew * cos(lastSinewR*lastSinewR);
		if (inputSampleR - lastSinewR > sinew) temp = lastSinewR + sinew;
		if (-(inputSampleR - lastSinewR) > sinew) temp = lastSinewR - sinew;
		lastSinewR = temp;
		inputSampleR = (inputSampleR * (1.0-depthSinew))+(lastSinewR*depthSinew);
		//run Sinew to stop excess slews, but run a dry/wet to allow a range of brights
		
		switch (dither) {
			case 1:
				//begin Dark		
				inputSampleL *= 8388608.0;
				inputSampleR *= 8388608.0; //we will apply the 24 bit Dark
				//We are doing it first Left, then Right, because the loops may run faster if
				//they aren't too jammed full of variables. This means re-running code.
				
				//begin left
				quantA = floor(inputSampleL);
				quantB = floor(inputSampleL+1.0);
				//to do this style of dither, we quantize in either direction and then
				//do a reconstruction of what the result will be for each choice.
				//We then evaluate which one we like, and keep a history of what we previously had
				
				expectedSlew = 0;
				for(int x = 0; x < depth; x++) {
					expectedSlew += (dram->darkSampleL[x+1] - dram->darkSampleL[x]);
				}
				expectedSlew /= depth; //we have an average of all recent slews
				//we are doing that to voice the thing down into the upper mids a bit
				//it mustn't just soften the brightest treble, it must smooth high mids too
				
				testA = fabs((dram->darkSampleL[0] - quantA) - expectedSlew);
				testB = fabs((dram->darkSampleL[0] - quantB) - expectedSlew);
				
				if (testA < testB) inputSampleL = quantA;
				else inputSampleL = quantB;
				//select whichever one departs LEAST from the vector of averaged
				//reconstructed previous final samples. This will force a kind of dithering
				//as it'll make the output end up as smooth as possible
				
				for(int x = depth; x >=0; x--) {
					dram->darkSampleL[x+1] = dram->darkSampleL[x];
				}
				dram->darkSampleL[0] = inputSampleL;
				//end Dark left
				
				//begin right
				quantA = floor(inputSampleR);
				quantB = floor(inputSampleR+1.0);
				//to do this style of dither, we quantize in either direction and then
				//do a reconstruction of what the result will be for each choice.
				//We then evaluate which one we like, and keep a history of what we previously had
				
				expectedSlew = 0;
				for(int x = 0; x < depth; x++) {
					expectedSlew += (dram->darkSampleR[x+1] - dram->darkSampleR[x]);
				}
				expectedSlew /= depth; //we have an average of all recent slews
				//we are doing that to voice the thing down into the upper mids a bit
				//it mustn't just soften the brightest treble, it must smooth high mids too
				
				testA = fabs((dram->darkSampleR[0] - quantA) - expectedSlew);
				testB = fabs((dram->darkSampleR[0] - quantB) - expectedSlew);
				
				if (testA < testB) inputSampleR = quantA;
				else inputSampleR = quantB;
				//select whichever one departs LEAST from the vector of averaged
				//reconstructed previous final samples. This will force a kind of dithering
				//as it'll make the output end up as smooth as possible
				
				for(int x = depth; x >=0; x--) {
					dram->darkSampleR[x+1] = dram->darkSampleR[x];
				}
				dram->darkSampleR[0] = inputSampleR;
				//end Dark right
				
				inputSampleL /= 8388608.0;
				inputSampleR /= 8388608.0;
				break; //Dark (Monitoring2)
			case 2:
				//begin Dark	 for Ten Nines
				inputSampleL *= 8388608.0;
				inputSampleR *= 8388608.0; //we will apply the 24 bit Dark
				//We are doing it first Left, then Right, because the loops may run faster if
				//they aren't too jammed full of variables. This means re-running code.
				
				//begin L
				correction = 0;
				if (flip) {
					NSOddL = (NSOddL * 0.9999999999) + prevShapeL;
					NSEvenL = (NSEvenL * 0.9999999999) - prevShapeL;
					correction = NSOddL;
				} else {
					NSOddL = (NSOddL * 0.9999999999) - prevShapeL;
					NSEvenL = (NSEvenL * 0.9999999999) + prevShapeL;
					correction = NSEvenL;
				}
				shapedSampleL = inputSampleL+correction;
				//end Ten Nines L
				
				//begin Dark L
				quantA = floor(shapedSampleL);
				quantB = floor(shapedSampleL+1.0);
				//to do this style of dither, we quantize in either direction and then
				//do a reconstruction of what the result will be for each choice.
				//We then evaluate which one we like, and keep a history of what we previously had
				
				expectedSlew = 0;
				for(int x = 0; x < depth; x++) {
					expectedSlew += (dram->darkSampleL[x+1] - dram->darkSampleL[x]);
				}
				expectedSlew /= depth; //we have an average of all recent slews
				//we are doing that to voice the thing down into the upper mids a bit
				//it mustn't just soften the brightest treble, it must smooth high mids too
				
				testA = fabs((dram->darkSampleL[0] - quantA) - expectedSlew);
				testB = fabs((dram->darkSampleL[0] - quantB) - expectedSlew);
				
				if (testA < testB) inputSampleL = quantA;
				else inputSampleL = quantB;
				//select whichever one departs LEAST from the vector of averaged
				//reconstructed previous final samples. This will force a kind of dithering
				//as it'll make the output end up as smooth as possible
				
				for(int x = depth; x >=0; x--) {
					dram->darkSampleL[x+1] = dram->darkSampleL[x];
				}
				dram->darkSampleL[0] = inputSampleL;
				//end Dark L
				
				prevShapeL = (floor(shapedSampleL) - inputSampleL)*0.9999999999;
				//end Ten Nines L
				
				//begin R
				correction = 0;
				if (flip) {
					NSOddR = (NSOddR * 0.9999999999) + prevShapeR;
					NSEvenR = (NSEvenR * 0.9999999999) - prevShapeR;
					correction = NSOddR;
				} else {
					NSOddR = (NSOddR * 0.9999999999) - prevShapeR;
					NSEvenR = (NSEvenR * 0.9999999999) + prevShapeR;
					correction = NSEvenR;
				}
				shapedSampleR = inputSampleR+correction;
				//end Ten Nines R
				
				//begin Dark R
				quantA = floor(shapedSampleR);
				quantB = floor(shapedSampleR+1.0);
				//to do this style of dither, we quantize in either direction and then
				//do a reconstruction of what the result will be for each choice.
				//We then evaluate which one we like, and keep a history of what we previously had
				
				expectedSlew = 0;
				for(int x = 0; x < depth; x++) {
					expectedSlew += (dram->darkSampleR[x+1] - dram->darkSampleR[x]);
				}
				expectedSlew /= depth; //we have an average of all recent slews
				//we are doing that to voice the thing down into the upper mids a bit
				//it mustn't just soften the brightest treble, it must smooth high mids too
				
				testA = fabs((dram->darkSampleR[0] - quantA) - expectedSlew);
				testB = fabs((dram->darkSampleR[0] - quantB) - expectedSlew);
				
				if (testA < testB) inputSampleR = quantA;
				else inputSampleR = quantB;
				//select whichever one departs LEAST from the vector of averaged
				//reconstructed previous final samples. This will force a kind of dithering
				//as it'll make the output end up as smooth as possible
				
				for(int x = depth; x >=0; x--) {
					dram->darkSampleR[x+1] = dram->darkSampleR[x];
				}
				dram->darkSampleR[0] = inputSampleR;
				//end Dark R
				
				prevShapeR = (floor(shapedSampleR) - inputSampleR)*0.9999999999;
				//end Ten Nines
				flip = !flip;
				
				inputSampleL /= 8388608.0;
				inputSampleR /= 8388608.0;
				break; //Ten Nines (which goes into Dark in Monitoring3)
			case 3:
				inputSampleL *= 8388608.0;
				inputSampleR *= 8388608.0;
				
				ditherL = -1.0;
				ditherL += (double(fpdL)/UINT32_MAX);
				fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
				ditherL += (double(fpdL)/UINT32_MAX);
				//TPDF: two 0-1 random noises
				
				ditherR = -1.0;
				ditherR += (double(fpdR)/UINT32_MAX);
				fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
				ditherR += (double(fpdR)/UINT32_MAX);
				//TPDF: two 0-1 random noises
				
				if (fabs(ditherL-ditherR) < 0.5) {
					ditherL = -1.0;
					ditherL += (double(fpdL)/UINT32_MAX);
					fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
					ditherL += (double(fpdL)/UINT32_MAX);
				}
				
				if (fabs(ditherL-ditherR) < 0.5) {
					ditherR = -1.0;
					ditherR += (double(fpdR)/UINT32_MAX);
					fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
					ditherR += (double(fpdR)/UINT32_MAX);
				}
				
				if (fabs(ditherL-ditherR) < 0.5) {
					ditherL = -1.0;
					ditherL += (double(fpdL)/UINT32_MAX);
					fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
					ditherL += (double(fpdL)/UINT32_MAX);
				}
				
				inputSampleL = floor(inputSampleL+ditherL);
				inputSampleR = floor(inputSampleR+ditherR);
				
				inputSampleL /= 8388608.0;
				inputSampleR /= 8388608.0;
				break; //TPDFWide (a good neutral with the width enhancement)
			case 4:
				inputSampleL *= 8388608.0;
				inputSampleR *= 8388608.0;
				//Paul Frindle: It's true that the dither itself can sound different 
				//if it's given a different freq response and you get to hear it. 
				//The one we use most is triangular single pole high pass dither. 
				//It's not freq bent enough to sound odd, but is slightly less audible than 
				//flat dither. It can also be easily made by taking one sample of dither 
				//away from the previous one - this gives you the triangular PDF and the 
				//filtering in one go :-)
				
				currentDither = (double(fpdL)/UINT32_MAX);
				ditherL = currentDither;
				ditherL -= previousDitherL;
				previousDitherL = currentDither;
				//TPDF: two 0-1 random noises
				
				currentDither = (double(fpdR)/UINT32_MAX);
				ditherR = currentDither;
				ditherR -= previousDitherR;
				previousDitherR = currentDither;
				//TPDF: two 0-1 random noises
				
				if (fabs(ditherL-ditherR) < 0.5) {
					fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
					currentDither = (double(fpdL)/UINT32_MAX);
					ditherL = currentDither;
					ditherL -= previousDitherL;
					previousDitherL = currentDither;
				}
				
				if (fabs(ditherL-ditherR) < 0.5) {
					fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
					currentDither = (double(fpdR)/UINT32_MAX);
					ditherR = currentDither;
					ditherR -= previousDitherR;
					previousDitherR = currentDither;
				}
				
				if (fabs(ditherL-ditherR) < 0.5) {
					fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
					currentDither = (double(fpdL)/UINT32_MAX);
					ditherL = currentDither;
					ditherL -= previousDitherL;
					previousDitherL = currentDither;
				}
				
				inputSampleL = floor(inputSampleL+ditherL);
				inputSampleR = floor(inputSampleR+ditherR);
				
				inputSampleL /= 8388608.0;
				inputSampleR /= 8388608.0;
				break; //PaulWide (brighter neutral that's still TPDF and wide)
			case 5:
				inputSampleL *= 8388608.0;
				inputSampleR *= 8388608.0;
				cutbinsL = false;
				cutbinsR = false;
				drySampleL = inputSampleL;//re-using in NJAD
				inputSampleL -= noiseShapingL;
				//NJAD L
				benfordize = floor(inputSampleL);
				while (benfordize >= 1.0) benfordize /= 10;
				while (benfordize < 1.0 && benfordize > 0.0000001) benfordize *= 10;
				hotbinA = floor(benfordize);
				//hotbin becomes the Benford bin value for this number floored
				totalA = 0.0;
				if ((hotbinA > 0) && (hotbinA < 10))
				{
					bynL[hotbinA] += 1; if (bynL[hotbinA] > 982) cutbinsL = true;
					totalA += (301-bynL[1]); totalA += (176-bynL[2]); totalA += (125-bynL[3]);
					totalA += (97-bynL[4]); totalA += (79-bynL[5]); totalA += (67-bynL[6]);
					totalA += (58-bynL[7]); totalA += (51-bynL[8]); totalA += (46-bynL[9]); bynL[hotbinA] -= 1;
				} else hotbinA = 10;
				//produce total number- smaller is closer to Benford real
				benfordize = ceil(inputSampleL);
				while (benfordize >= 1.0) benfordize /= 10;
				while (benfordize < 1.0 && benfordize > 0.0000001) benfordize *= 10;
				hotbinB = floor(benfordize);
				//hotbin becomes the Benford bin value for this number ceiled
				totalB = 0.0;
				if ((hotbinB > 0) && (hotbinB < 10))
				{
					bynL[hotbinB] += 1; if (bynL[hotbinB] > 982) cutbinsL = true;
					totalB += (301-bynL[1]); totalB += (176-bynL[2]); totalB += (125-bynL[3]);
					totalB += (97-bynL[4]); totalB += (79-bynL[5]); totalB += (67-bynL[6]);
					totalB += (58-bynL[7]); totalB += (51-bynL[8]); totalB += (46-bynL[9]); bynL[hotbinB] -= 1;
				} else hotbinB = 10;
				//produce total number- smaller is closer to Benford real
				if (totalA < totalB) {bynL[hotbinA] += 1; outputSample = floor(inputSampleL);}
				else {bynL[hotbinB] += 1; outputSample = floor(inputSampleL+1);}
				//assign the relevant one to the delay line
				//and floor/ceil signal accordingly
				if (cutbinsL) {
					bynL[1] *= 0.99; bynL[2] *= 0.99; bynL[3] *= 0.99; bynL[4] *= 0.99; bynL[5] *= 0.99; 
					bynL[6] *= 0.99; bynL[7] *= 0.99; bynL[8] *= 0.99; bynL[9] *= 0.99; bynL[10] *= 0.99; 
				}
				noiseShapingL += outputSample - drySampleL;			
				if (noiseShapingL > fabs(inputSampleL)) noiseShapingL = fabs(inputSampleL);
				if (noiseShapingL < -fabs(inputSampleL)) noiseShapingL = -fabs(inputSampleL);
				inputSampleL /= 8388608.0;
				if (inputSampleL > 1.0) inputSampleL = 1.0;
				if (inputSampleL < -1.0) inputSampleL = -1.0;
				//finished NJAD L
				
				//NJAD R
				drySampleR = inputSampleR;
				inputSampleR -= noiseShapingR;
				benfordize = floor(inputSampleR);
				while (benfordize >= 1.0) benfordize /= 10;
				while (benfordize < 1.0 && benfordize > 0.0000001) benfordize *= 10;		
				hotbinA = floor(benfordize);
				//hotbin becomes the Benford bin value for this number floored
				totalA = 0.0;
				if ((hotbinA > 0) && (hotbinA < 10))
				{
					bynR[hotbinA] += 1; if (bynR[hotbinA] > 982) cutbinsR = true;
					totalA += (301-bynR[1]); totalA += (176-bynR[2]); totalA += (125-bynR[3]);
					totalA += (97-bynR[4]); totalA += (79-bynR[5]); totalA += (67-bynR[6]);
					totalA += (58-bynR[7]); totalA += (51-bynR[8]); totalA += (46-bynR[9]); bynR[hotbinA] -= 1;
				} else hotbinA = 10;
				//produce total number- smaller is closer to Benford real
				benfordize = ceil(inputSampleR);
				while (benfordize >= 1.0) benfordize /= 10;
				while (benfordize < 1.0 && benfordize > 0.0000001) benfordize *= 10;		
				hotbinB = floor(benfordize);
				//hotbin becomes the Benford bin value for this number ceiled
				totalB = 0.0;
				if ((hotbinB > 0) && (hotbinB < 10))
				{
					bynR[hotbinB] += 1; if (bynR[hotbinB] > 982) cutbinsR = true;
					totalB += (301-bynR[1]); totalB += (176-bynR[2]); totalB += (125-bynR[3]);
					totalB += (97-bynR[4]); totalB += (79-bynR[5]); totalB += (67-bynR[6]);
					totalB += (58-bynR[7]); totalB += (51-bynR[8]); totalB += (46-bynR[9]); bynR[hotbinB] -= 1;
				} else hotbinB = 10;
				//produce total number- smaller is closer to Benford real
				if (totalA < totalB) {bynR[hotbinA] += 1; outputSample = floor(inputSampleR);}
				else {bynR[hotbinB] += 1; outputSample = floor(inputSampleR+1);}
				//assign the relevant one to the delay line
				//and floor/ceil signal accordingly
				if (cutbinsR) {
					bynR[1] *= 0.99; bynR[2] *= 0.99; bynR[3] *= 0.99; bynR[4] *= 0.99; bynR[5] *= 0.99; 
					bynR[6] *= 0.99; bynR[7] *= 0.99; bynR[8] *= 0.99; bynR[9] *= 0.99; bynR[10] *= 0.99; 
				}
				noiseShapingR += outputSample - drySampleR;			
				if (noiseShapingR > fabs(inputSampleR)) noiseShapingR = fabs(inputSampleR);
				if (noiseShapingR < -fabs(inputSampleR)) noiseShapingR = -fabs(inputSampleR);
				inputSampleR /= 8388608.0;
				if (inputSampleR > 1.0) inputSampleR = 1.0;
				if (inputSampleR < -1.0) inputSampleR = -1.0;				
				break; //NJAD (Monitoring. Brightest)
			case 6:
				//begin 32 bit stereo floating point dither
				frexpf((float)inputSampleL, &expon);
				fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
				inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
				frexpf((float)inputSampleR, &expon);
				fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
				inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
				//end 32 bit stereo floating point dither
				break; //Bypass for saving floating point files directly
		}
		
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
	for (int x = 0; x < air_total; x++) dram->air[x] = 0.0;
	for (int x = 0; x < kal_total; x++) {dram->kalM[x] = 0.0;dram->kalS[x] = 0.0;}
	
	lastSinewL = 0.0;
	lastSinewR = 0.0;
	
	lastSampleL = 0.0;
	wasPosClipL = false;
	wasNegClipL = false;
	lastSampleR = 0.0;
	wasPosClipR = false;
	wasNegClipR = false;
	for (int x = 0; x < 16; x++) {intermediateL[x] = 0.0; intermediateR[x] = 0.0;}
	
	quantA = 0;
	quantB = 1;
	expectedSlew = 0.0;
	testA = 0.0;
	testB = 0.0;
	correction = 0.0;
	shapedSampleL = 0.0;
	shapedSampleR = 0.0;
	currentDither = 0.0;
	ditherL = 0.0;
	ditherR = 0.0;
	cutbinsL = false;
	cutbinsR = false;
	hotbinA = 0;
	hotbinB = 0;
	benfordize = 0.0;
	totalA = 0.0;
	totalB = 0.0;
	outputSample = 0.0;
	expon = 0; //internal dither variables
	//these didn't like to be defined inside a case statement
	
	NSOddL = 0.0; NSEvenL = 0.0; prevShapeL = 0.0;
	NSOddR = 0.0; NSEvenR = 0.0; prevShapeR = 0.0;
	flip = true; //Ten Nines
	for(int count = 0; count < 99; count++) {
		dram->darkSampleL[count] = 0;
		dram->darkSampleR[count] = 0;
	} //Dark
	previousDitherL = 0.0;
	previousDitherR = 0.0; //PaulWide
	bynL[0] = 1000.0;
	bynL[1] = 301.0;
	bynL[2] = 176.0;
	bynL[3] = 125.0;
	bynL[4] = 97.0;
	bynL[5] = 79.0;
	bynL[6] = 67.0;
	bynL[7] = 58.0;
	bynL[8] = 51.0;
	bynL[9] = 46.0;
	bynL[10] = 1000.0;
	noiseShapingL = 0.0;
	bynR[0] = 1000.0;
	bynR[1] = 301.0;
	bynR[2] = 176.0;
	bynR[3] = 125.0;
	bynR[4] = 97.0;
	bynR[5] = 79.0;
	bynR[6] = 67.0;
	bynR[7] = 58.0;
	bynR[8] = 51.0;
	bynR[9] = 46.0;
	bynR[10] = 1000.0;
	noiseShapingR = 0.0; //NJAD
	
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	return noErr;
}

};

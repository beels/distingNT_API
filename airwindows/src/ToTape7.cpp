#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "ToTape7"
#define AIRWINDOWS_DESCRIPTION "Airwindows tape emulation with Bias and Dubly."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','T','o','V' )
enum {

	kParam_A =0,
	kParam_B =1,
	kParam_C =2,
	kParam_D =3,
	kParam_E =4,
	kParam_F =5,
	kParam_G =6,
	kParam_H =7,
	kParam_I =8,
	kParam_J =9,
	//Add your parameters here...
	kNumberOfParameters=10
};
enum { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode,
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, kParam8, kParam9, };
static const uint8_t page2[] = { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input L", 1, 1 )
NT_PARAMETER_AUDIO_INPUT( "Input R", 1, 2 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output L", 1, 13 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output R", 1, 14 )
{ .name = "EncAmt", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "EncFreq", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "TapeDrv", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Flutter", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "FlutSpd", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Bias", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "HeadBmp", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "HeadFrq", .min = 2500, .max = 20000, .def = 5000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL },
{ .name = "DecAmt", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "DecFreq", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, kParam8, kParam9, };
enum { kNumTemplateParameters = 6 };
#include "../include/template1.h"
 
	
	double iirEncL;
	double iirEncR;

	double dL[1002];
	double dR[1002];
	double sweepL;
	double sweepR;
	double nextmaxL;
	double nextmaxR;
	int gcount;		
	
	enum {
		prevSampL1,
		prevSampR1,
		threshold1,
		prevSampL2,
		prevSampR2,
		threshold2,
		prevSampL3,
		prevSampR3,
		threshold3,
		prevSampL4,
		prevSampR4,
		threshold4,
		prevSampL5,
		prevSampR5,
		threshold5,
		prevSampL6,
		prevSampR6,
		threshold6,
		prevSampL7,
		prevSampR7,
		threshold7,
		prevSampL8,
		prevSampR8,
		threshold8,
		prevSampL9,
		prevSampR9,
		threshold9,
		gslew_total
	}; //fixed frequency pear filter for ultrasonics, stereo
	double gslew[gslew_total]; //end bias	
	
	double iirMidRollerL;
	double iirLowCutoffL;
	double iirMidRollerR;
	double iirLowCutoffR;
	
	double headBumpL;
	double headBumpR;
	enum {
		hdb_freq,
		hdb_reso,
		hdb_a0,
		hdb_a1,
		hdb_a2,
		hdb_b1,
		hdb_b2,
		hdb_sL1,
		hdb_sL2,
		hdb_sR1,
		hdb_sR2,
		hdb_total
	}; //fixed frequency biquad filter for ultrasonics, stereo
	double hdbA[hdb_total];
	double hdbB[hdb_total];
	
	double iirDecL;
	double iirDecR;
	
	double lastSampleL;
	double intermediateL[16];
	bool wasPosClipL;
	bool wasNegClipL;
	double lastSampleR;
	double intermediateR[16];
	bool wasPosClipR;
	bool wasNegClipR; //Stereo ClipOnly2	
	
	uint32_t fpdL;
	uint32_t fpdR;
#include "../include/template2.h"
#include "../include/templateStereo.h"
void _airwindowsAlgorithm::render( const Float32* inputL, const Float32* inputR, Float32* outputL, Float32* outputR, UInt32 inFramesToProcess ) {

	UInt32 nSampleFrames = inFramesToProcess;
	double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	
	int spacing = floor(overallscale); //should give us working basic scaling, usually 2 or 4
	if (spacing < 1) spacing = 1; if (spacing > 16) spacing = 16;

	double dublyAmount = pow(GetParameter( kParam_A ),3)*0.105468;
	double iirEncFreq = GetParameter( kParam_B )/overallscale;
	double iirMidFreq = ((GetParameter( kParam_B ) * 0.618) + 0.382)/overallscale;
	double inputGain = pow(GetParameter( kParam_C )*2.0,2.0);
	double flutDepth = pow(GetParameter( kParam_D ),6)*overallscale*50;
	if (flutDepth > 498.0) flutDepth = 498.0;
	double flutFrequency = (0.02*pow(GetParameter( kParam_E ),3))/overallscale;
	double bias = (GetParameter( kParam_F )*2.0)-1.0;
	double underBias = (pow(bias,4)*0.25)/overallscale;
	double overBias = pow(1.0-bias,3)/overallscale;
	if (bias > 0.0) underBias = 0.0;
	if (bias < 0.0) overBias = 1.0/overallscale;
	
	gslew[threshold9] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold8] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold7] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold6] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold5] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold4] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold3] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold2] = overBias;
	overBias *= 1.618033988749894848204586;
	gslew[threshold1] = overBias;
	overBias *= 1.618033988749894848204586;
	
	double headBumpDrive = (GetParameter( kParam_G )*0.1)/overallscale;
	double headBumpMix = GetParameter( kParam_G )*0.5;
	
	hdbA[hdb_freq] = GetParameter( kParam_H )/GetSampleRate();
	hdbB[hdb_freq] = hdbA[hdb_freq]*0.9375;
	hdbB[hdb_reso] = hdbA[hdb_reso] = 0.618033988749894848204586;
	hdbB[hdb_a1] = hdbA[hdb_a1] = 0.0;
	
	double K = tan(M_PI * hdbA[hdb_freq]);
	double norm = 1.0 / (1.0 + K / hdbA[hdb_reso] + K * K);
	hdbA[hdb_a0] = K / hdbA[hdb_reso] * norm;
	hdbA[hdb_a2] = -hdbA[hdb_a0];
	hdbA[hdb_b1] = 2.0 * (K * K - 1.0) * norm;
	hdbA[hdb_b2] = (1.0 - K / hdbA[hdb_reso] + K * K) * norm;
	K = tan(M_PI * hdbB[hdb_freq]);
	norm = 1.0 / (1.0 + K / hdbB[hdb_reso] + K * K);
	hdbB[hdb_a0] = K / hdbB[hdb_reso] * norm;
	hdbB[hdb_a2] = -hdbB[hdb_a0];
	hdbB[hdb_b1] = 2.0 * (K * K - 1.0) * norm;
	hdbB[hdb_b2] = (1.0 - K / hdbB[hdb_reso] + K * K) * norm;

	double outlyAmount = pow(GetParameter( kParam_I ),3)*0.109744;
	double iirDecFreq = GetParameter( kParam_J )/overallscale;
	double subCurve = sin(GetParameter( kParam_G )*M_PI);
	double iirSubFreq = (subCurve*0.008)/overallscale;
	
	while (nSampleFrames-- > 0) {
		double inputSampleL = *inputL;
		double inputSampleR = *inputR;
		if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;
		
		//begin Dubly encode
		double doubly = 0.0;
		if (dublyAmount > 0.0) {
			iirEncL = (iirEncL * (1.0 - iirEncFreq)) + (inputSampleL * iirEncFreq);
			double doubly = inputSampleL - iirEncL;
			if (doubly > 1.0) doubly = 1.0; if (doubly < -1.0) doubly = -1.0;
			if (doubly > 0) doubly = log(1.0+(255*fabs(doubly)))/2.40823996531;
			if (doubly < 0) doubly = -log(1.0+(255*fabs(doubly)))/2.40823996531;
			inputSampleL += doubly*dublyAmount;
			iirEncR = (iirEncR * (1.0 - iirEncFreq)) + (inputSampleR * iirEncFreq);
			doubly = inputSampleR - iirEncR;
			if (doubly > 1.0) doubly = 1.0; if (doubly < -1.0) doubly = -1.0;
			if (doubly > 0) doubly = log(1.0+(255*fabs(doubly)))/2.40823996531;
			if (doubly < 0) doubly = -log(1.0+(255*fabs(doubly)))/2.40823996531;
			inputSampleR += doubly*dublyAmount;
		}
		//end Dubly encode
		
		if (inputGain != 1.0) {
			inputSampleL *= inputGain;
			inputSampleR *= inputGain;
		}
		
		//begin Flutter
		if (flutDepth > 0.0) {
			if (gcount < 0 || gcount > 999) gcount = 999;
			dL[gcount] = inputSampleL;
			int count = gcount;
			double offset = flutDepth + (flutDepth * sin(sweepL));
			sweepL += nextmaxL * flutFrequency;
			if (sweepL > (M_PI*2.0)) {
				sweepL -= M_PI*2.0;
				double flutA = 0.24 + (fpdL / (double)UINT32_MAX * 0.74);
				fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
				double flutB = 0.24 + (fpdL / (double)UINT32_MAX * 0.74);
				if (fabs(flutA-sin(sweepR+nextmaxR))<fabs(flutB-sin(sweepR+nextmaxR))) nextmaxL = flutA; else nextmaxL = flutB;
			}
			count += (int)floor(offset);
			inputSampleL = (dL[count-((count > 999)?1000:0)] * (1-(offset-floor(offset))));
			inputSampleL += (dL[count+1-((count+1 > 999)?1000:0)] * (offset-floor(offset)));
			dR[gcount] = inputSampleR;
			count = gcount;
			offset = flutDepth + (flutDepth * sin(sweepR));
			sweepR += nextmaxR * flutFrequency;
			if (sweepR > (M_PI*2.0)) {
				sweepR -= M_PI*2.0;
				double flutA = 0.24 + (fpdR / (double)UINT32_MAX * 0.74);
				fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
				double flutB = 0.24 + (fpdR / (double)UINT32_MAX * 0.74);
				if (fabs(flutA-sin(sweepL+nextmaxL))<fabs(flutB-sin(sweepL+nextmaxL))) nextmaxR = flutA; else nextmaxR = flutB;
			}
			count += (int)floor(offset);
			inputSampleR = (dR[count-((count > 999)?1000:0)] * (1-(offset-floor(offset))));
			inputSampleR += (dR[count+1-((count+1 > 999)?1000:0)] * (offset-floor(offset)));
			gcount--;
		}
		//end Flutter
		
		//start bias routine
		if (fabs(bias) > 0.001) {
			for (int x = 0; x < gslew_total; x += 3) {
				if (underBias > 0.0) {
					double stuck = fabs(inputSampleL - (gslew[x]/0.975)) / underBias;
					if (stuck < 1.0) inputSampleL = (inputSampleL * stuck) + ((gslew[x]/0.975)*(1.0-stuck));
					stuck =  fabs(inputSampleR - (gslew[x+1]/0.975)) / underBias;
					if (stuck < 1.0) inputSampleR = (inputSampleR * stuck) + ((gslew[x+1]/0.975)*(1.0-stuck));
				}
				if ((inputSampleL - gslew[x]) > gslew[x+2]) inputSampleL = gslew[x] + gslew[x+2];
				if (-(inputSampleL - gslew[x]) > gslew[x+2]) inputSampleL = gslew[x] - gslew[x+2];
				gslew[x] = inputSampleL * 0.975;
				if ((inputSampleR - gslew[x+1]) > gslew[x+2]) inputSampleR = gslew[x+1] + gslew[x+2];
				if (-(inputSampleR - gslew[x+1]) > gslew[x+2]) inputSampleR = gslew[x+1] - gslew[x+2];
				gslew[x+1] = inputSampleR * 0.975;
			}
		}
		//end bias routine
		
		//toTape basic algorithm L
		iirMidRollerL = (iirMidRollerL * (1.0-iirMidFreq)) + (inputSampleL*iirMidFreq);
		double HighsSampleL = inputSampleL - iirMidRollerL;
		double LowsSampleL = iirMidRollerL;
		if (iirSubFreq > 0.0) {
			iirLowCutoffL = (iirLowCutoffL * (1.0-iirSubFreq)) + (LowsSampleL*iirSubFreq);
			LowsSampleL -= iirLowCutoffL;
		}
		if (LowsSampleL > 1.57079633) LowsSampleL = 1.57079633;
		if (LowsSampleL < -1.57079633) LowsSampleL = -1.57079633;
		LowsSampleL = sin(LowsSampleL);
		double thinnedHighSample = fabs(HighsSampleL)*1.57079633;
		if (thinnedHighSample > 1.57079633) thinnedHighSample = 1.57079633;
		thinnedHighSample = 1.0-cos(thinnedHighSample);
		if (HighsSampleL < 0) thinnedHighSample = -thinnedHighSample;
		HighsSampleL -= thinnedHighSample;
		
		//toTape basic algorithm R
		iirMidRollerR = (iirMidRollerR * (1.0-iirMidFreq)) + (inputSampleR*iirMidFreq);
		double HighsSampleR = inputSampleR - iirMidRollerR;
		double LowsSampleR = iirMidRollerR;
		if (iirSubFreq > 0.0) {
			iirLowCutoffR = (iirLowCutoffR * (1.0-iirSubFreq)) + (LowsSampleR*iirSubFreq);
			LowsSampleR -= iirLowCutoffR;
		}
		if (LowsSampleR > 1.57079633) LowsSampleR = 1.57079633;
		if (LowsSampleR < -1.57079633) LowsSampleR = -1.57079633;
		LowsSampleR = sin(LowsSampleR);
		thinnedHighSample = fabs(HighsSampleR)*1.57079633;
		if (thinnedHighSample > 1.57079633) thinnedHighSample = 1.57079633;
		thinnedHighSample = 1.0-cos(thinnedHighSample);
		if (HighsSampleR < 0) thinnedHighSample = -thinnedHighSample;
		HighsSampleR -= thinnedHighSample;
		
		//begin HeadBump
		double headBumpSampleL = 0.0;
		double headBumpSampleR = 0.0;
		if (headBumpMix > 0.0) {
			headBumpL += (LowsSampleL * headBumpDrive);
			headBumpL -= (headBumpL * headBumpL * headBumpL * (0.0618/sqrt(overallscale)));
			headBumpR += (LowsSampleR * headBumpDrive);
			headBumpR -= (headBumpR * headBumpR * headBumpR * (0.0618/sqrt(overallscale)));
			double headBiqSampleL = (headBumpL * hdbA[hdb_a0]) + hdbA[hdb_sL1];
			hdbA[hdb_sL1] = (headBumpL * hdbA[hdb_a1]) - (headBiqSampleL * hdbA[hdb_b1]) + hdbA[hdb_sL2];
			hdbA[hdb_sL2] = (headBumpL * hdbA[hdb_a2]) - (headBiqSampleL * hdbA[hdb_b2]);
			headBumpSampleL = (headBiqSampleL * hdbB[hdb_a0]) + hdbB[hdb_sL1];
			hdbB[hdb_sL1] = (headBiqSampleL * hdbB[hdb_a1]) - (headBumpSampleL * hdbB[hdb_b1]) + hdbB[hdb_sL2];
			hdbB[hdb_sL2] = (headBiqSampleL * hdbB[hdb_a2]) - (headBumpSampleL * hdbB[hdb_b2]);
			double headBiqSampleR = (headBumpR * hdbA[hdb_a0]) + hdbA[hdb_sR1];
			hdbA[hdb_sR1] = (headBumpR * hdbA[hdb_a1]) - (headBiqSampleR * hdbA[hdb_b1]) + hdbA[hdb_sR2];
			hdbA[hdb_sR2] = (headBumpR * hdbA[hdb_a2]) - (headBiqSampleR * hdbA[hdb_b2]);
			headBumpSampleR = (headBiqSampleR * hdbB[hdb_a0]) + hdbB[hdb_sR1];
			hdbB[hdb_sR1] = (headBiqSampleR * hdbB[hdb_a1]) - (headBumpSampleR * hdbB[hdb_b1]) + hdbB[hdb_sR2];
			hdbB[hdb_sR2] = (headBiqSampleR * hdbB[hdb_a2]) - (headBumpSampleR * hdbB[hdb_b2]);
		}
		//end HeadBump
		
		inputSampleL = LowsSampleL + HighsSampleL + (headBumpSampleL * headBumpMix);
		inputSampleR = LowsSampleR + HighsSampleR + (headBumpSampleR * headBumpMix);
		
		//begin Dubly decode
		if (outlyAmount > 0.0) {
			iirDecL = (iirDecL * (1.0 - iirDecFreq)) + (inputSampleL * iirDecFreq);
			doubly = inputSampleL - iirDecL;
			if (doubly > 1.0) doubly = 1.0; if (doubly < -1.0) doubly = -1.0;
			if (doubly > 0) doubly = log(1.0+(255*fabs(doubly)))/2.40823996531;
			if (doubly < 0) doubly = -log(1.0+(255*fabs(doubly)))/2.40823996531;
			inputSampleL -= doubly*outlyAmount;
			iirDecR = (iirDecR * (1.0 - iirDecFreq)) + (inputSampleR * iirDecFreq);
			doubly = inputSampleR - iirDecR;
			if (doubly > 1.0) doubly = 1.0; if (doubly < -1.0) doubly = -1.0;
			if (doubly > 0) doubly = log(1.0+(255*fabs(doubly)))/2.40823996531;
			if (doubly < 0) doubly = -log(1.0+(255*fabs(doubly)))/2.40823996531;
			inputSampleR -= doubly*outlyAmount;
		}
		//end Dubly decode

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
		
		//begin 32 bit stereo floating point dither
		int expon; frexpf((float)inputSampleL, &expon);
		fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
		inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
		frexpf((float)inputSampleR, &expon);
		fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
		inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
		//end 32 bit stereo floating point dither
		
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
	iirEncL = 0.0;
	iirEncR = 0.0;
	
	for (int temp = 0; temp < 1001; temp++) {dL[temp] = 0.0;dR[temp] = 0.0;}
	sweepL = M_PI;
	sweepR = M_PI;
	nextmaxL = 0.5;	
	nextmaxR = 0.5;
	gcount = 0;	
	
	for (int x = 0; x < gslew_total; x++) gslew[x] = 0.0;

	iirMidRollerL = 0.0;
	iirLowCutoffL = 0.0;
	iirMidRollerR = 0.0;
	iirLowCutoffR = 0.0;

	headBumpL = 0.0;
	headBumpR = 0.0;
	for (int x = 0; x < hdb_total; x++) {hdbA[x] = 0.0;hdbB[x] = 0.0;}
	//from ZBandpass, so I can use enums with it	
	
	iirDecL = 0.0;
	iirDecR = 0.0;
	
	lastSampleL = 0.0;
	wasPosClipL = false;
	wasNegClipL = false;
	lastSampleR = 0.0;
	wasPosClipR = false;
	wasNegClipR = false;
	for (int x = 0; x < 16; x++) {intermediateL[x] = 0.0; intermediateR[x] = 0.0;}
	//this is reset: values being initialized only once. Startup values, whatever they are.
	
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	return noErr;
}

};

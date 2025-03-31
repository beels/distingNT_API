#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "MV "
#define AIRWINDOWS_DESCRIPTION "A dual-mono reverb based on BitShiftGain and the old Midiverbs."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','M','V',' ' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	kParam_Five =4,
	//Add your parameters here...
	kNumberOfParameters=5
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParam0, kParam1, kParam2, kParam3, kParam4, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Depth", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Bright", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Regen", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Output", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, };
enum { kNumTemplateParameters = 3 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
		
		Float64 aA[15150];
		Float64 aB[14618];
		Float64 aC[14358];
		Float64 aD[13818];		
		Float64 aE[13562];
		Float64 aF[13046];
		Float64 aG[11966];
		Float64 aH[11130];
		Float64 aI[10598];
		Float64 aJ[9810];
		Float64 aK[9522];
		Float64 aL[8982];
		Float64 aM[8786];
		Float64 aN[8462];
		Float64 aO[8310];
		Float64 aP[7982];
		Float64 aQ[7322];
		Float64 aR[6818];
		Float64 aS[6506];
		Float64 aT[6002];
		Float64 aU[5838];
		Float64 aV[5502];
		Float64 aW[5010];
		Float64 aX[4850];
		Float64 aY[4296];
		Float64 aZ[4180];
				
		int alpA, delayA;
		int alpB, delayB;
		int alpC, delayC;
		int alpD, delayD;
		int alpE, delayE;
		int alpF, delayF;
		int alpG, delayG;
		int alpH, delayH;
		int alpI, delayI;
		int alpJ, delayJ;
		int alpK, delayK;
		int alpL, delayL;
		int alpM, delayM;
		int alpN, delayN;
		int alpO, delayO;
		int alpP, delayP;
		int alpQ, delayQ;
		int alpR, delayR;
		int alpS, delayS;
		int alpT, delayT;
		int alpU, delayU;
		int alpV, delayV;
		int alpW, delayW;
		int alpX, delayX;
		int alpY, delayY;
		int alpZ, delayZ;
		
		Float64 avgA;
		Float64 avgB;
		Float64 avgC;
		Float64 avgD;
		Float64 avgE;
		Float64 avgF;
		Float64 avgG;
		Float64 avgH;
		Float64 avgI;
		Float64 avgJ;
		Float64 avgK;
		Float64 avgL;
		Float64 avgM;
		Float64 avgN;
		Float64 avgO;
		Float64 avgP;
		Float64 avgQ;
		Float64 avgR;
		Float64 avgS;
		Float64 avgT;
		Float64 avgU;
		Float64 avgV;
		Float64 avgW;
		Float64 avgX;
		Float64 avgY;
		Float64 avgZ;
		
		Float64 feedback;
		
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
	int allpasstemp;
	Float64 avgtemp;
	int stage = GetParameter( kParam_One ) * 27.0;
	int damp = (1.0-GetParameter( kParam_Two )) * stage;
	Float64 feedbacklevel = GetParameter( kParam_Three );
	if (feedbacklevel <= 0.0625) feedbacklevel = 0.0;
	if (feedbacklevel > 0.0625 && feedbacklevel <= 0.125) feedbacklevel = 0.0625; //-24db
	if (feedbacklevel > 0.125 && feedbacklevel <= 0.25) feedbacklevel = 0.125; //-18db
	if (feedbacklevel > 0.25 && feedbacklevel <= 0.5) feedbacklevel = 0.25; //-12db
	if (feedbacklevel > 0.5 && feedbacklevel <= 0.99) feedbacklevel = 0.5; //-6db
	if (feedbacklevel > 0.99) feedbacklevel = 1.0;
	//we're forcing even the feedback level to be Midiverb-ized
	Float64 gain = GetParameter( kParam_Four );
	Float64 wet = GetParameter( kParam_Five );
	
	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;

		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;
		double drySample = inputSample;
		
		inputSample += feedback;
		
		inputSample = sin(inputSample);

		
		switch (stage){
			case 27:
			case 26:
				allpasstemp = alpA - 1;
				if (allpasstemp < 0 || allpasstemp > delayA) {allpasstemp = delayA;}
				inputSample -= aA[allpasstemp]*0.5;
				aA[alpA] = inputSample;
				inputSample *= 0.5;
				alpA--; if (alpA < 0 || alpA > delayA) {alpA = delayA;}
				inputSample += (aA[alpA]);
				if (damp > 26) {
				avgtemp = inputSample;
				inputSample += avgA;
				inputSample *= 0.5;
				avgA = avgtemp;
				}
				//allpass filter A		
			case 25:
				allpasstemp = alpB - 1;
				if (allpasstemp < 0 || allpasstemp > delayB) {allpasstemp = delayB;}
				inputSample -= aB[allpasstemp]*0.5;
				aB[alpB] = inputSample;
				inputSample *= 0.5;
				alpB--; if (alpB < 0 || alpB > delayB) {alpB = delayB;}
				inputSample += (aB[alpB]);
				if (damp > 25) {
				avgtemp = inputSample;
				inputSample += avgB;
				inputSample *= 0.5;
				avgB = avgtemp;
				}
				//allpass filter B
			case 24:
				allpasstemp = alpC - 1;
				if (allpasstemp < 0 || allpasstemp > delayC) {allpasstemp = delayC;}
				inputSample -= aC[allpasstemp]*0.5;
				aC[alpC] = inputSample;
				inputSample *= 0.5;
				alpC--; if (alpC < 0 || alpC > delayC) {alpC = delayC;}
				inputSample += (aC[alpC]);
				if (damp > 24) {
				avgtemp = inputSample;
				inputSample += avgC;
				inputSample *= 0.5;
				avgC = avgtemp;
				}
				//allpass filter C
			case 23:
				allpasstemp = alpD - 1;
				if (allpasstemp < 0 || allpasstemp > delayD) {allpasstemp = delayD;}
				inputSample -= aD[allpasstemp]*0.5;
				aD[alpD] = inputSample;
				inputSample *= 0.5;
				alpD--; if (alpD < 0 || alpD > delayD) {alpD = delayD;}
				inputSample += (aD[alpD]);
				if (damp > 23) {
				avgtemp = inputSample;
				inputSample += avgD;
				inputSample *= 0.5;
				avgD = avgtemp;
				}
				//allpass filter D
			case 22:
				allpasstemp = alpE - 1;
				if (allpasstemp < 0 || allpasstemp > delayE) {allpasstemp = delayE;}
				inputSample -= aE[allpasstemp]*0.5;
				aE[alpE] = inputSample;
				inputSample *= 0.5;
				alpE--; if (alpE < 0 || alpE > delayE) {alpE = delayE;}
				inputSample += (aE[alpE]);
				if (damp > 22) {
				avgtemp = inputSample;
				inputSample += avgE;
				inputSample *= 0.5;
				avgE = avgtemp;
				}
				//allpass filter E
			case 21:
				allpasstemp = alpF - 1;
				if (allpasstemp < 0 || allpasstemp > delayF) {allpasstemp = delayF;}
				inputSample -= aF[allpasstemp]*0.5;
				aF[alpF] = inputSample;
				inputSample *= 0.5;
				alpF--; if (alpF < 0 || alpF > delayF) {alpF = delayF;}
				inputSample += (aF[alpF]);
				if (damp > 21) {
				avgtemp = inputSample;
				inputSample += avgF;
				inputSample *= 0.5;
				avgF = avgtemp;
				}
				//allpass filter F
			case 20:
				allpasstemp = alpG - 1;
				if (allpasstemp < 0 || allpasstemp > delayG) {allpasstemp = delayG;}
				inputSample -= aG[allpasstemp]*0.5;
				aG[alpG] = inputSample;
				inputSample *= 0.5;
				alpG--; if (alpG < 0 || alpG > delayG) {alpG = delayG;}
				inputSample += (aG[alpG]);
				if (damp > 20) {
				avgtemp = inputSample;
				inputSample += avgG;
				inputSample *= 0.5;
				avgG = avgtemp;
				}
				//allpass filter G
			case 19:
				allpasstemp = alpH - 1;
				if (allpasstemp < 0 || allpasstemp > delayH) {allpasstemp = delayH;}
				inputSample -= aH[allpasstemp]*0.5;
				aH[alpH] = inputSample;
				inputSample *= 0.5;
				alpH--; if (alpH < 0 || alpH > delayH) {alpH = delayH;}
				inputSample += (aH[alpH]);
				if (damp > 19) {
				avgtemp = inputSample;
				inputSample += avgH;
				inputSample *= 0.5;
				avgH = avgtemp;
				}
				//allpass filter H
			case 18:
				allpasstemp = alpI - 1;
				if (allpasstemp < 0 || allpasstemp > delayI) {allpasstemp = delayI;}
				inputSample -= aI[allpasstemp]*0.5;
				aI[alpI] = inputSample;
				inputSample *= 0.5;
				alpI--; if (alpI < 0 || alpI > delayI) {alpI = delayI;}
				inputSample += (aI[alpI]);
				if (damp > 18) {
				avgtemp = inputSample;
				inputSample += avgI;
				inputSample *= 0.5;
				avgI = avgtemp;
				}
				//allpass filter I
			case 17:
				allpasstemp = alpJ - 1;
				if (allpasstemp < 0 || allpasstemp > delayJ) {allpasstemp = delayJ;}
				inputSample -= aJ[allpasstemp]*0.5;
				aJ[alpJ] = inputSample;
				inputSample *= 0.5;
				alpJ--; if (alpJ < 0 || alpJ > delayJ) {alpJ = delayJ;}
				inputSample += (aJ[alpJ]);
				if (damp > 17) {
				avgtemp = inputSample;
				inputSample += avgJ;
				inputSample *= 0.5;
				avgJ = avgtemp;
				}
				//allpass filter J
			case 16:
				allpasstemp = alpK - 1;
				if (allpasstemp < 0 || allpasstemp > delayK) {allpasstemp = delayK;}
				inputSample -= aK[allpasstemp]*0.5;
				aK[alpK] = inputSample;
				inputSample *= 0.5;
				alpK--; if (alpK < 0 || alpK > delayK) {alpK = delayK;}
				inputSample += (aK[alpK]);
				if (damp > 16) {
				avgtemp = inputSample;
				inputSample += avgK;
				inputSample *= 0.5;
				avgK = avgtemp;
				}
				//allpass filter K
			case 15:
				allpasstemp = alpL - 1;
				if (allpasstemp < 0 || allpasstemp > delayL) {allpasstemp = delayL;}
				inputSample -= aL[allpasstemp]*0.5;
				aL[alpL] = inputSample;
				inputSample *= 0.5;
				alpL--; if (alpL < 0 || alpL > delayL) {alpL = delayL;}
				inputSample += (aL[alpL]);
				if (damp > 15) {
				avgtemp = inputSample;
				inputSample += avgL;
				inputSample *= 0.5;
				avgL = avgtemp;
				}
				//allpass filter L
			case 14:
				allpasstemp = alpM - 1;
				if (allpasstemp < 0 || allpasstemp > delayM) {allpasstemp = delayM;}
				inputSample -= aM[allpasstemp]*0.5;
				aM[alpM] = inputSample;
				inputSample *= 0.5;
				alpM--; if (alpM < 0 || alpM > delayM) {alpM = delayM;}
				inputSample += (aM[alpM]);
				if (damp > 14) {
				avgtemp = inputSample;
				inputSample += avgM;
				inputSample *= 0.5;
				avgM = avgtemp;
				}
				//allpass filter M
			case 13:
				allpasstemp = alpN - 1;
				if (allpasstemp < 0 || allpasstemp > delayN) {allpasstemp = delayN;}
				inputSample -= aN[allpasstemp]*0.5;
				aN[alpN] = inputSample;
				inputSample *= 0.5;
				alpN--; if (alpN < 0 || alpN > delayN) {alpN = delayN;}
				inputSample += (aN[alpN]);
				if (damp > 13) {
				avgtemp = inputSample;
				inputSample += avgN;
				inputSample *= 0.5;
				avgN = avgtemp;
				}
				//allpass filter N
			case 12:
				allpasstemp = alpO - 1;
				if (allpasstemp < 0 || allpasstemp > delayO) {allpasstemp = delayO;}
				inputSample -= aO[allpasstemp]*0.5;
				aO[alpO] = inputSample;
				inputSample *= 0.5;
				alpO--; if (alpO < 0 || alpO > delayO) {alpO = delayO;}
				inputSample += (aO[alpO]);
				if (damp > 12) {
				avgtemp = inputSample;
				inputSample += avgO;
				inputSample *= 0.5;
				avgO = avgtemp;
				}
				//allpass filter O
			case 11:
				allpasstemp = alpP - 1;
				if (allpasstemp < 0 || allpasstemp > delayP) {allpasstemp = delayP;}
				inputSample -= aP[allpasstemp]*0.5;
				aP[alpP] = inputSample;
				inputSample *= 0.5;
				alpP--; if (alpP < 0 || alpP > delayP) {alpP = delayP;}
				inputSample += (aP[alpP]);
				if (damp > 11) {
				avgtemp = inputSample;
				inputSample += avgP;
				inputSample *= 0.5;
				avgP = avgtemp;
				}
				//allpass filter P
			case 10:
				allpasstemp = alpQ - 1;
				if (allpasstemp < 0 || allpasstemp > delayQ) {allpasstemp = delayQ;}
				inputSample -= aQ[allpasstemp]*0.5;
				aQ[alpQ] = inputSample;
				inputSample *= 0.5;
				alpQ--; if (alpQ < 0 || alpQ > delayQ) {alpQ = delayQ;}
				inputSample += (aQ[alpQ]);
				if (damp > 10) {
				avgtemp = inputSample;
				inputSample += avgQ;
				inputSample *= 0.5;
				avgQ = avgtemp;
				}
				//allpass filter Q
			case 9:
				allpasstemp = alpR - 1;
				if (allpasstemp < 0 || allpasstemp > delayR) {allpasstemp = delayR;}
				inputSample -= aR[allpasstemp]*0.5;
				aR[alpR] = inputSample;
				inputSample *= 0.5;
				alpR--; if (alpR < 0 || alpR > delayR) {alpR = delayR;}
				inputSample += (aR[alpR]);
				if (damp > 9) {
				avgtemp = inputSample;
				inputSample += avgR;
				inputSample *= 0.5;
				avgR = avgtemp;
				}
				//allpass filter R
			case 8:
				allpasstemp = alpS - 1;
				if (allpasstemp < 0 || allpasstemp > delayS) {allpasstemp = delayS;}
				inputSample -= aS[allpasstemp]*0.5;
				aS[alpS] = inputSample;
				inputSample *= 0.5;
				alpS--; if (alpS < 0 || alpS > delayS) {alpS = delayS;}
				inputSample += (aS[alpS]);
				if (damp > 8) {
				avgtemp = inputSample;
				inputSample += avgS;
				inputSample *= 0.5;
				avgS = avgtemp;
				}
				//allpass filter S
			case 7:
				allpasstemp = alpT - 1;
				if (allpasstemp < 0 || allpasstemp > delayT) {allpasstemp = delayT;}
				inputSample -= aT[allpasstemp]*0.5;
				aT[alpT] = inputSample;
				inputSample *= 0.5;
				alpT--; if (alpT < 0 || alpT > delayT) {alpT = delayT;}
				inputSample += (aT[alpT]);
				if (damp > 7) {
				avgtemp = inputSample;
				inputSample += avgT;
				inputSample *= 0.5;
				avgT = avgtemp;
				}
				//allpass filter T
			case 6:
				allpasstemp = alpU - 1;
				if (allpasstemp < 0 || allpasstemp > delayU) {allpasstemp = delayU;}
				inputSample -= aU[allpasstemp]*0.5;
				aU[alpU] = inputSample;
				inputSample *= 0.5;
				alpU--; if (alpU < 0 || alpU > delayU) {alpU = delayU;}
				inputSample += (aU[alpU]);
				if (damp > 6) {
				avgtemp = inputSample;
				inputSample += avgU;
				inputSample *= 0.5;
				avgU = avgtemp;
				}
				//allpass filter U
			case 5:
				allpasstemp = alpV - 1;
				if (allpasstemp < 0 || allpasstemp > delayV) {allpasstemp = delayV;}
				inputSample -= aV[allpasstemp]*0.5;
				aV[alpV] = inputSample;
				inputSample *= 0.5;
				alpV--; if (alpV < 0 || alpV > delayV) {alpV = delayV;}
				inputSample += (aV[alpV]);
				if (damp > 5) {
				avgtemp = inputSample;
				inputSample += avgV;
				inputSample *= 0.5;
				avgV = avgtemp;
				}
				//allpass filter V
			case 4:
				allpasstemp = alpW - 1;
				if (allpasstemp < 0 || allpasstemp > delayW) {allpasstemp = delayW;}
				inputSample -= aW[allpasstemp]*0.5;
				aW[alpW] = inputSample;
				inputSample *= 0.5;
				alpW--; if (alpW < 0 || alpW > delayW) {alpW = delayW;}
				inputSample += (aW[alpW]);
				if (damp > 4) {
				avgtemp = inputSample;
				inputSample += avgW;
				inputSample *= 0.5;
				avgW = avgtemp;
				}
				//allpass filter W
			case 3:
				allpasstemp = alpX - 1;
				if (allpasstemp < 0 || allpasstemp > delayX) {allpasstemp = delayX;}
				inputSample -= aX[allpasstemp]*0.5;
				aX[alpX] = inputSample;
				inputSample *= 0.5;
				alpX--; if (alpX < 0 || alpX > delayX) {alpX = delayX;}
				inputSample += (aX[alpX]);
				if (damp > 3) {
				avgtemp = inputSample;
				inputSample += avgX;
				inputSample *= 0.5;
				avgX = avgtemp;
				}
				//allpass filter X
			case 2:
				allpasstemp = alpY - 1;
				if (allpasstemp < 0 || allpasstemp > delayY) {allpasstemp = delayY;}
				inputSample -= aY[allpasstemp]*0.5;
				aY[alpY] = inputSample;
				inputSample *= 0.5;
				alpY--; if (alpY < 0 || alpY > delayY) {alpY = delayY;}
				inputSample += (aY[alpY]);
				if (damp > 2) {
				avgtemp = inputSample;
				inputSample += avgY;
				inputSample *= 0.5;
				avgY = avgtemp;
				}
				//allpass filter Y
			case 1:
				allpasstemp = alpZ - 1;
				if (allpasstemp < 0 || allpasstemp > delayZ) {allpasstemp = delayZ;}
				inputSample -= aZ[allpasstemp]*0.5;
				aZ[alpZ] = inputSample;
				inputSample *= 0.5;
				alpZ--; if (alpZ < 0 || alpZ > delayZ) {alpZ = delayZ;}
				inputSample += (aZ[alpZ]);
				if (damp > 1) {
				avgtemp = inputSample;
				inputSample += avgZ;
				inputSample *= 0.5;
				avgZ = avgtemp;
				}
				//allpass filter Z
		}
		
		feedback = inputSample * feedbacklevel;
				
		if (gain != 1.0) {
			inputSample *= gain;
		}
		//we can pad with the gain to tame distortyness from the PurestConsole code
		
		if (inputSample > 1.0) inputSample = 1.0;
		if (inputSample < -1.0) inputSample = -1.0;
		//without this, you can get a NaN condition where it spits out DC offset at full blast!
		
		inputSample = asin(inputSample);
		
		
		if (wet !=1.0) {
			inputSample = (inputSample * wet) + (drySample * (1.0-wet));
		}
		//Dry/Wet control, defaults to the last slider

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
	int count;
	for(count = 0; count < 15149; count++) {aA[count] = 0.0;}
	for(count = 0; count < 14617; count++) {aB[count] = 0.0;}
	for(count = 0; count < 14357; count++) {aC[count] = 0.0;}
	for(count = 0; count < 13817; count++) {aD[count] = 0.0;}
	for(count = 0; count < 13561; count++) {aE[count] = 0.0;}
	for(count = 0; count < 13045; count++) {aF[count] = 0.0;}
	for(count = 0; count < 11965; count++) {aG[count] = 0.0;}
	for(count = 0; count < 11129; count++) {aH[count] = 0.0;}
	for(count = 0; count < 10597; count++) {aI[count] = 0.0;}
	for(count = 0; count < 9809; count++) {aJ[count] = 0.0;}
	for(count = 0; count < 9521; count++) {aK[count] = 0.0;}
	for(count = 0; count < 8981; count++) {aL[count] = 0.0;}
	for(count = 0; count < 8785; count++) {aM[count] = 0.0;}
	for(count = 0; count < 8461; count++) {aN[count] = 0.0;}
	for(count = 0; count < 8309; count++) {aO[count] = 0.0;}
	for(count = 0; count < 7981; count++) {aP[count] = 0.0;}
	for(count = 0; count < 7321; count++) {aQ[count] = 0.0;}
	for(count = 0; count < 6817; count++) {aR[count] = 0.0;}
	for(count = 0; count < 6505; count++) {aS[count] = 0.0;}
	for(count = 0; count < 6001; count++) {aT[count] = 0.0;}
	for(count = 0; count < 5837; count++) {aU[count] = 0.0;}
	for(count = 0; count < 5501; count++) {aV[count] = 0.0;}
	for(count = 0; count < 5009; count++) {aW[count] = 0.0;}
	for(count = 0; count < 4849; count++) {aX[count] = 0.0;}
	for(count = 0; count < 4295; count++) {aY[count] = 0.0;}
	for(count = 0; count < 4179; count++) {aZ[count] = 0.0;}	
	
	alpA = 1; delayA = 7573; avgA = 0.0;
	alpB = 1; delayB = 7307; avgB = 0.0;
	alpC = 1; delayC = 7177; avgC = 0.0;
	alpD = 1; delayD = 6907; avgD = 0.0;
	alpE = 1; delayE = 6779; avgE = 0.0;
	alpF = 1; delayF = 6521; avgF = 0.0;
	alpG = 1; delayG = 5981; avgG = 0.0;
	alpH = 1; delayH = 5563; avgH = 0.0;
	alpI = 1; delayI = 5297; avgI = 0.0;
	alpJ = 1; delayJ = 4903; avgJ = 0.0;
	alpK = 1; delayK = 4759; avgK = 0.0;
	alpL = 1; delayL = 4489; avgL = 0.0;
	alpM = 1; delayM = 4391; avgM = 0.0;
	alpN = 1; delayN = 4229; avgN = 0.0;
	alpO = 1; delayO = 4153; avgO = 0.0;
	alpP = 1; delayP = 3989; avgP = 0.0;
	alpQ = 1; delayQ = 3659; avgQ = 0.0;
	alpR = 1; delayR = 3407; avgR = 0.0;
	alpS = 1; delayS = 3251; avgS = 0.0;
	alpT = 1; delayT = 2999; avgT = 0.0;
	alpU = 1; delayU = 2917; avgU = 0.0;
	alpV = 1; delayV = 2749; avgV = 0.0;
	alpW = 1; delayW = 2503; avgW = 0.0;
	alpX = 1; delayX = 2423; avgX = 0.0;
	alpY = 1; delayY = 2146; avgY = 0.0;
	alpZ = 1; delayZ = 2088; avgZ = 0.0;
	
	feedback = 0.0;
	
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};

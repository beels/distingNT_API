#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "CansAW"
#define AIRWINDOWS_DESCRIPTION "Chris's Airwindows room, in a plugin, for headphone mixers."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','C','a','o' )
enum {

	kParam_A =0,
	//Add your parameters here...
	kNumberOfParameters=1
};
static const int kA = 0;
static const int kB = 1;
static const int kC = 2;
static const int kD = 3;
static const int kDefaultValue_ParamA = kB;
const int kshortA = 193;
const int kshortB = 464;
const int kshortC = 422;
const int kshortD = 36;
const int kshortE = 166;
const int kshortF = 427;
const int kshortG = 162;
const int kshortH = 476;
const int kshortI = 23;
const int kshortJ = 345;
const int kshortK = 298;
const int kshortL = 296;
const int kshortM = 12;
const int kshortN = 989;
const int kshortO = 65;
const int kshortP = 484;
enum { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode,
kParam0, };
static char const * const enumStrings0[] = { "Bypass", "UpFront", "SitBack", "Hallway", };
static const uint8_t page2[] = { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input L", 1, 1 )
NT_PARAMETER_AUDIO_INPUT( "Input R", 1, 2 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output L", 1, 13 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output R", 1, 14 )
{ .name = "seatPos", .min = 0, .max = 3, .def = 1, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = enumStrings0 },
};
static const uint8_t page1[] = {
kParam0, };
enum { kNumTemplateParameters = 6 };
#include "../include/template1.h"
 
	double aAL[kshortA+5];
	double aBL[kshortB+5];
	double aCL[kshortC+5];
	double aDL[kshortD+5];
	double aEL[kshortE+5];
	double aFL[kshortF+5];
	double aGL[kshortG+5];
	double aHL[kshortH+5];
	double aIL[kshortI+5];
	double aJL[kshortJ+5];
	double aKL[kshortK+5];
	double aLL[kshortL+5];
	double aML[kshortM+5];
	double aNL[kshortN+5];
	double aOL[kshortO+5];
	double aPL[kshortP+5];
	
	double aAR[kshortA+5];
	double aBR[kshortB+5];
	double aCR[kshortC+5];
	double aDR[kshortD+5];
	double aER[kshortE+5];
	double aFR[kshortF+5];
	double aGR[kshortG+5];
	double aHR[kshortH+5];
	double aIR[kshortI+5];
	double aJR[kshortJ+5];
	double aKR[kshortK+5];
	double aLR[kshortL+5];
	double aMR[kshortM+5];
	double aNR[kshortN+5];
	double aOR[kshortO+5];
	double aPR[kshortP+5];
	
	double feedbackAL;
	double feedbackBL;
	double feedbackCL;
	double feedbackDL;
	double feedbackAR;
	double feedbackBR;
	double feedbackCR;
	double feedbackDR;
	
	double iirInL;
	double iirFAL;
	double iirFBL;
	double iirFCL;
	double iirFDL;
	double iirOutL;
	double iirInR;
	double iirFAR;
	double iirFBR;
	double iirFCR;
	double iirFDR;
	double iirOutR;
	
	int countA;
	int countB;
	int countC;
	int countD;
	int countE;
	int countF;
	int countG;
	int countH;
	int countI;
	int countJ;
	int countK;
	int countL;		
	int countM;		
	int countN;		
	int countO;		
	int countP;		
	
	int shortA;
	int shortB;
	int shortC;
	int shortD;
	int shortE;
	int shortF;
	int shortG;
	int shortH;
	int shortI;
	int shortJ;
	int shortK;
	int shortL;
	int shortM;
	int shortN;
	int shortO;
	int shortP;
		
	enum {
		bez_AL,
		bez_AR,
		bez_BL,
		bez_BR,
		bez_CL,
		bez_CR,	
		bez_InL,
		bez_InR,
		bez_UnInL,
		bez_UnInR,
		bez_SampL,
		bez_SampR,
		bez_cycle,
		bez_total
	}; //the new undersampling. bez signifies the bezier curve reconstruction
	double bez[bez_total];
	
	uint32_t fpdL;
	uint32_t fpdR;
#include "../include/template2.h"
#include "../include/templateStereo.h"
void _airwindowsAlgorithm::render( const Float32* inputL, const Float32* inputR, Float32* outputL, Float32* outputR, UInt32 inFramesToProcess ) {

	UInt32 nSampleFrames = inFramesToProcess;
	double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	
	int roomPos = GetParameter( kParam_A );
	double derez = 1.0/overallscale;
	derez = 1.0 / ((int)(1.0/derez));
	//this hard-locks derez to exact subdivisions of 1.0
	double paramC = 0.5;
	double paramD = 0.5;
	double paramE = 0.5;
	switch (roomPos)
	{
		case 0:
			 paramC = 0.538307;
			 paramD = 0.183726;
			 paramE = 0.0;
			break;
		case 1:
			 paramC = 0.538307;
			 paramD = 0.183726;
			 paramE = 0.319380;
			break;
		case 2:
			 paramC = 0.640114;
			 paramD = 0.366477;
			 paramE = 0.395308;
			break;
		case 3:
		default:
			 paramC = 0.308438;
			 paramD = 0.318292;
			 paramE = 0.853204;
			break;
		//this sets up our presets using the existing code
	}			
	
	int diffuse = pow(0.894020,1.618033988749894848204586)*16;
	double iirAmt = 1.0-pow(1.0-paramC,1.618033988749894848204586);
	double iirExt = iirAmt/overallscale;
	double crossfade = (1.0-pow(1.0-paramD,1.618033988749894848204586));
	double wet = pow(paramE,1.618033988749894848204586);
	
	while (nSampleFrames-- > 0) {
		double inputSampleL = *inputL;
		double inputSampleR = *inputR;
		if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;
		double drySampleL = inputSampleL;
		double drySampleR = inputSampleR;
		
		iirInL = (inputSampleL*iirExt)+(iirInL*(1.0-iirExt)); inputSampleL = iirInL;
		iirInR = (inputSampleR*iirExt)+(iirInR*(1.0-iirExt)); inputSampleR = iirInR;
		
		bez[bez_cycle] += derez;
		bez[bez_SampL] += ((inputSampleL+bez[bez_InL]) * derez);
		bez[bez_SampR] += ((inputSampleR+bez[bez_InR]) * derez);
		bez[bez_InL] = inputSampleL; bez[bez_InR] = inputSampleR;
		if (bez[bez_cycle] > 1.0) { //hit the end point and we do a reverb sample
			bez[bez_cycle] = 0.0;
			
			iirFAL = (feedbackAL*iirAmt)+(iirFAL*(1.0-iirAmt)); feedbackAL = iirFAL;
			iirFAR = (feedbackAR*iirAmt)+(iirFAR*(1.0-iirAmt)); feedbackAR = iirFAR;
			iirFBL = (feedbackBL*iirAmt)+(iirFBL*(1.0-iirAmt)); feedbackBL = iirFBL;
			iirFBR = (feedbackBR*iirAmt)+(iirFBR*(1.0-iirAmt)); feedbackBR = iirFBR;
			iirFCL = (feedbackCL*iirAmt)+(iirFCL*(1.0-iirAmt)); feedbackCL = iirFCL;
			iirFCR = (feedbackCR*iirAmt)+(iirFCR*(1.0-iirAmt)); feedbackCR = iirFCR;
			iirFDL = (feedbackDL*iirAmt)+(iirFDL*(1.0-iirAmt)); feedbackDL = iirFDL;
			iirFDR = (feedbackDR*iirAmt)+(iirFDR*(1.0-iirAmt)); feedbackDR = iirFDR;
			
			double outAL, outBL, outCL, outDL, outAR, outBR, outCR, outDR;
			if (diffuse < 4) {
				aAL[countA] = (bez[bez_SampL]) + (feedbackAL * 0.03125);
				aAR[countA] = (bez[bez_SampR]) + (feedbackAR * 0.03125);
				countA++; if (countA < 0 || countA > shortA) countA = 0;
				outAL = aAL[countA-((countA > shortA)?shortA+1:0)];
				outAR = aAR[countA-((countA > shortA)?shortA+1:0)];
			} else {
				outAL = (bez[bez_SampL] - (aAL[(countA+1)-((countA+1 > shortA)?shortA+1:0)]*0.618033988749894848204586)) + (feedbackAL * 0.03125);
				aAL[countA] = outAL; outAL *= 0.618033988749894848204586;
				outAR = (bez[bez_SampR] - (aAR[(countA+1)-((countA+1 > shortA)?shortA+1:0)]*0.618033988749894848204586)) + (feedbackAR * 0.03125);
				aAR[countA] = outAR; outAR *= 0.618033988749894848204586;
				countA++; if (countA < 0 || countA > shortA) countA = 0;
				outAL += aAL[countA-((countA > shortA)?shortA+1:0)];
				outAR += aAR[countA-((countA > shortA)?shortA+1:0)];
			}
			
			if (diffuse < 12) {
				aBL[countB] = (bez[bez_SampL]) + (feedbackBL * 0.03125);
				aBR[countB] = (bez[bez_SampR]) + (feedbackBR * 0.03125);
				countB++; if (countB < 0 || countB > shortB) countB = 0;
				outBL = aBL[countB-((countB > shortB)?shortB+1:0)];
				outBR = aBR[countB-((countB > shortB)?shortB+1:0)];
			} else {
				outBL = (bez[bez_SampL] - (aBL[(countB+1)-((countB+1 > shortB)?shortB+1:0)]*0.618033988749894848204586)) + (feedbackBL * 0.03125);
				aBL[countB] = outBL; outBL *= 0.618033988749894848204586;
				outBR = (bez[bez_SampR] - (aBR[(countB+1)-((countB+1 > shortB)?shortB+1:0)]*0.618033988749894848204586)) + (feedbackBR * 0.03125);
				aBR[countB] = outBR; outBR *= 0.618033988749894848204586;
				countB++; if (countB < 0 || countB > shortB) countB = 0;
				outBL += aBL[countB-((countB > shortB)?shortB+1:0)];
				outBR += aBR[countB-((countB > shortB)?shortB+1:0)];
			}
			
			if (diffuse < 10) {
				aCL[countC] = (bez[bez_SampL]) + (feedbackCL * 0.03125);
				aCR[countC] = (bez[bez_SampR]) + (feedbackCR * 0.03125);
				countC++; if (countC < 0 || countC > shortC) countC = 0;
				outCL = aCL[countC-((countC > shortC)?shortC+1:0)];
				outCR = aCR[countC-((countC > shortC)?shortC+1:0)];
			} else {
				outCL = (bez[bez_SampL] - (aCL[(countC+1)-((countC+1 > shortC)?shortC+1:0)]*0.618033988749894848204586)) + (feedbackCL * 0.03125);
				aCL[countC] = outCL; outCL *= 0.618033988749894848204586;
				outCR = (bez[bez_SampR] - (aCR[(countC+1)-((countC+1 > shortC)?shortC+1:0)]*0.618033988749894848204586)) + (feedbackCR * 0.03125);
				aCR[countC] = outCR; outCR *= 0.618033988749894848204586;
				countC++; if (countC < 0 || countC > shortC) countC = 0;
				outCL += aCL[countC-((countC > shortC)?shortC+1:0)];
				outCR += aCR[countC-((countC > shortC)?shortC+1:0)];
			}
			
			if (diffuse < 6) {
				aDL[countD] = (bez[bez_SampL]) + (feedbackDL * 0.03125);
				aDR[countD] = (bez[bez_SampR]) + (feedbackDR * 0.03125);
				countD++; if (countD < 0 || countD > shortD) countD = 0;
				outDL = aDL[countD-((countD > shortD)?shortD+1:0)];
				outDR = aDR[countD-((countD > shortD)?shortD+1:0)];
			} else {
				outDL = (bez[bez_SampL] - (aDL[(countD+1)-((countD+1 > shortD)?shortD+1:0)]*0.618033988749894848204586)) + (feedbackDL * 0.03125);
				aDL[countD] = outDL; outDL *= 0.618033988749894848204586;
				outDR = (bez[bez_SampR] - (aDR[(countD+1)-((countD+1 > shortD)?shortD+1:0)]*0.618033988749894848204586)) + (feedbackDR * 0.03125);
				aDR[countD] = outDR; outDR *= 0.618033988749894848204586;
				countD++; if (countD < 0 || countD > shortD) countD = 0;
				outDL += aDL[countD-((countD > shortD)?shortD+1:0)];
				outDR += aDR[countD-((countD > shortD)?shortD+1:0)];
			}
			
			double outEL, outFL, outGL, outHL, outER, outFR, outGR, outHR;
			if (diffuse < 3) {
				aEL[countE] = outAL - (outBL + outCL + outDL);
				aER[countE] = outAR - (outBR + outCR + outDR);
				countE++; if (countE < 0 || countE > shortE) countE = 0;
				outEL = aEL[countE-((countE > shortE)?shortE+1:0)];
				outER = aER[countE-((countE > shortE)?shortE+1:0)];
			} else {
				outEL = (outAL - (outBL + outCL + outDL)) - (aEL[(countE+1)-((countE+1 > shortE)?shortE+1:0)]*0.618033988749894848204586);
				aEL[countE] = outEL; outEL *= 0.618033988749894848204586;
				outER = (outAR - (outBR + outCR + outDR)) - (aER[(countE+1)-((countE+1 > shortE)?shortE+1:0)]*0.618033988749894848204586);
				aER[countE] = outER; outER *= 0.618033988749894848204586;
				countE++; if (countE < 0 || countE > shortE) countE = 0;
				outEL += aEL[countE-((countE > shortE)?shortE+1:0)];
				outER += aER[countE-((countE > shortE)?shortE+1:0)];
			}
			
			if (diffuse < 8) {
				aFL[countF] = outBL - (outAL + outCL + outDL);
				aFR[countF] = outBR - (outAR + outCR + outDR);
				countF++; if (countF < 0 || countF > shortF) countF = 0;
				outFL = aFL[countF-((countF > shortF)?shortF+1:0)];
				outFR = aFR[countF-((countF > shortF)?shortF+1:0)];
			} else {
				outFL = (outBL - (outAL + outCL + outDL)) - (aFL[(countF+1)-((countF+1 > shortF)?shortF+1:0)]*0.618033988749894848204586);
				aFL[countF] = outFL; outFL *= 0.618033988749894848204586;
				outFR = (outBR - (outAR + outCR + outDR)) - (aFR[(countF+1)-((countF+1 > shortF)?shortF+1:0)]*0.618033988749894848204586);
				aFR[countF] = outFR; outFR *= 0.618033988749894848204586;
				countF++; if (countF < 0 || countF > shortF) countF = 0;
				outFL += aFL[countF-((countF > shortF)?shortF+1:0)];
				outFR += aFR[countF-((countF > shortF)?shortF+1:0)];
			}
			
			if (diffuse < 5) {
				aGL[countG] = outCL - (outAL + outBL + outDL);
				aGR[countG] = outCR - (outAR + outBR + outDR);
				countG++; if (countG < 0 || countG > shortG) countG = 0;
				outGL = aGL[countG-((countG > shortG)?shortG+1:0)];
				outGR = aGR[countG-((countG > shortG)?shortG+1:0)];
			} else {
				outGL = (outCL - (outAL + outBL + outDL)) - (aGL[(countG+1)-((countG+1 > shortG)?shortG+1:0)]*0.618033988749894848204586);
				aGL[countG] = outGL; outGL *= 0.618033988749894848204586;
				outGR = (outCR - (outAR + outBR + outDR)) - (aGR[(countG+1)-((countG+1 > shortG)?shortG+1:0)]*0.618033988749894848204586);
				aGR[countG] = outGR; outGR *= 0.618033988749894848204586;
				countG++; if (countG < 0 || countG > shortG) countG = 0;
				outGL += aGL[countG-((countG > shortG)?shortG+1:0)];
				outGR += aGR[countG-((countG > shortG)?shortG+1:0)];
			}
			
			if (diffuse < 11) {
				aHL[countH] = outDL - (outAL + outBL + outCL);
				aHR[countH] = outDR - (outAR + outBR + outCR);
				countH++; if (countH < 0 || countH > shortH) countH = 0;
				outHL = aHL[countH-((countH > shortH)?shortH+1:0)];
				outHR = aHR[countH-((countH > shortH)?shortH+1:0)];
			} else {
				outHL = (outDL - (outAL + outBL + outCL)) - (aHL[(countH+1)-((countH+1 > shortH)?shortH+1:0)]*0.618033988749894848204586);
				aHL[countH] = outHL; outHL *= 0.618033988749894848204586;
				outHR = (outDR - (outAR + outBR + outCR)) - (aHR[(countH+1)-((countH+1 > shortH)?shortH+1:0)]*0.618033988749894848204586);
				aHR[countH] = outHR; outHR *= 0.618033988749894848204586;
				countH++; if (countH < 0 || countH > shortH) countH = 0;
				outHL += aHL[countH-((countH > shortH)?shortH+1:0)];
				outHR += aHR[countH-((countH > shortH)?shortH+1:0)];
			}
			
			double outIL, outJL, outKL, outLL, outIR, outJR, outKR, outLR;
			if (diffuse < 2) {
				aIL[countI] = outEL - (outFL + outGL + outHL);
				aIR[countI] = outER - (outFR + outGR + outHR);
				countI++; if (countI < 0 || countI > shortI) countI = 0;
				outIL = aIL[countI-((countI > shortI)?shortI+1:0)];
				outIR = aIR[countI-((countI > shortI)?shortI+1:0)];
			} else {
				outIL = (outEL - (outFL + outGL + outHL)) - (aIL[(countI+1)-((countI+1 > shortI)?shortI+1:0)]*0.618033988749894848204586);
				aIL[countI] = outIL; outIL *= 0.618033988749894848204586;
				outIR = (outER - (outFR + outGR + outHR)) - (aIR[(countI+1)-((countI+1 > shortI)?shortI+1:0)]*0.618033988749894848204586);
				aIR[countI] = outIR; outIR *= 0.618033988749894848204586;
				countI++; if (countI < 0 || countI > shortI) countI = 0;
				outIL += aIL[countI-((countI > shortI)?shortI+1:0)];
				outIR += aIR[countI-((countI > shortI)?shortI+1:0)];
			}
			
			if (diffuse < 14) {
				aJL[countJ] = outFL - (outEL + outGL + outHL);
				aJR[countJ] = outFR - (outER + outGR + outHR);
				countJ++; if (countJ < 0 || countJ > shortJ) countJ = 0;
				outJL = aJL[countJ-((countJ > shortJ)?shortJ+1:0)];
				outJR = aJR[countJ-((countJ > shortJ)?shortJ+1:0)];
			} else {
				outJL = (outFL - (outEL + outGL + outHL)) - (aJL[(countJ+1)-((countJ+1 > shortJ)?shortJ+1:0)]*0.618033988749894848204586);
				aJL[countJ] = outJL; outJL *= 0.618033988749894848204586;
				outJR = (outFR - (outER + outGR + outHR)) - (aJR[(countJ+1)-((countJ+1 > shortJ)?shortJ+1:0)]*0.618033988749894848204586);
				aJR[countJ] = outJR; outJR *= 0.618033988749894848204586;
				countJ++; if (countJ < 0 || countJ > shortJ) countJ = 0;
				outJL += aJL[countJ-((countJ > shortJ)?shortJ+1:0)];
				outJR += aJR[countJ-((countJ > shortJ)?shortJ+1:0)];
			}
			
			if (diffuse < 13) {
				aKL[countK] = outGL - (outEL + outFL + outHL);
				aKR[countK] = outGR - (outER + outFR + outHR);
				countK++; if (countK < 0 || countK > shortK) countK = 0;
				outKL = aKL[countK-((countK > shortK)?shortK+1:0)];
				outKR = aKR[countK-((countK > shortK)?shortK+1:0)];
			} else {
				outKL = (outGL - (outEL + outFL + outHL)) - (aKL[(countK+1)-((countK+1 > shortK)?shortK+1:0)]*0.618033988749894848204586);
				aKL[countK] = outKL; outKL *= 0.618033988749894848204586;
				outKR = (outGR - (outER + outFR + outHR)) - (aKR[(countK+1)-((countK+1 > shortK)?shortK+1:0)]*0.618033988749894848204586);
				aKR[countK] = outKR; outKR *= 0.618033988749894848204586;
				countK++; if (countK < 0 || countK > shortK) countK = 0;
				outKL += aKL[countK-((countK > shortK)?shortK+1:0)];
				outKR += aKR[countK-((countK > shortK)?shortK+1:0)];
			}
			
			if (diffuse < 7) {
				aLL[countL] = outHL - (outEL + outFL + outGL);
				aLR[countL] = outHR - (outER + outFR + outGR);
				countL++; if (countL < 0 || countL > shortL) countL = 0;
				outLL = aLL[countL-((countL > shortL)?shortL+1:0)];
				outLR = aLR[countL-((countL > shortL)?shortL+1:0)];
			} else {
				outLL = (outHL - (outEL + outFL + outGL)) - (aLL[(countL+1)-((countL+1 > shortL)?shortL+1:0)]*0.618033988749894848204586);
				aLL[countL] = outLL; outLL *= 0.618033988749894848204586;
				outLR = (outHR - (outER + outFR + outGR)) - (aLR[(countL+1)-((countL+1 > shortL)?shortL+1:0)]*0.618033988749894848204586);
				aLR[countL] = outLR; outLR *= 0.618033988749894848204586;
				countL++; if (countL < 0 || countL > shortL) countL = 0;
				outLL += aLL[countL-((countL > shortL)?shortL+1:0)];
				outLR += aLR[countL-((countL > shortL)?shortL+1:0)];
			}
			
			double outML, outNL, outOL, outPL, outMR, outNR, outOR, outPR;
			if (diffuse < 0) {
				aML[countM] = outIL - (outJL + outKL + outLL);
				aMR[countM] = outIR - (outJR + outKR + outLR);
				countM++; if (countM < 0 || countM > shortM) countM = 0;
				outML = aML[countM-((countM > shortM)?shortM+1:0)];
				outMR = aMR[countM-((countM > shortM)?shortM+1:0)];
			} else {
				outML = (outIL - (outJL + outKL + outLL)) - (aML[(countM+1)-((countM+1 > shortM)?shortM+1:0)]*0.618033988749894848204586);
				aML[countM] = outML; outML *= 0.618033988749894848204586;
				outMR = (outIR - (outJR + outKR + outLR)) - (aMR[(countM+1)-((countM+1 > shortM)?shortM+1:0)]*0.618033988749894848204586);
				aMR[countM] = outMR; outMR *= 0.618033988749894848204586;
				countM++; if (countM < 0 || countM > shortM) countM = 0;
				outML += aML[countM-((countM > shortM)?shortM+1:0)];
				outMR += aMR[countM-((countM > shortM)?shortM+1:0)];
			}
			
			if (diffuse < 15) {
				aNL[countN] = outJL - (outIL + outKL + outLL);
				aNR[countN] = outJR - (outIR + outKR + outLR);
				countN++; if (countN < 0 || countN > shortN) countN = 0;
				outNL = aNL[countN-((countN > shortN)?shortN+1:0)];
				outNR = aNR[countN-((countN > shortN)?shortN+1:0)];
			} else {
				outNL = (outJL - (outIL + outKL + outLL)) - (aNL[(countN+1)-((countN+1 > shortN)?shortN+1:0)]*0.618033988749894848204586);
				aNL[countN] = outNL; outNL *= 0.618033988749894848204586;
				outNR = (outJR - (outIR + outKR + outLR)) - (aNR[(countN+1)-((countN+1 > shortN)?shortN+1:0)]*0.618033988749894848204586);
				aNR[countN] = outNR; outNR *= 0.618033988749894848204586;
				countN++; if (countN < 0 || countN > shortN) countN = 0;
				outNL += aNL[countN-((countN > shortN)?shortN+1:0)];
				outNR += aNR[countN-((countN > shortN)?shortN+1:0)];
			}
			
			if (diffuse < 1) {
				aOL[countO] = outKL - (outIL + outJL + outLL);
				aOR[countO] = outKR - (outIR + outJR + outLR);
				countO++; if (countO < 0 || countO > shortO) countO = 0;
				outOL = aOL[countO-((countO > shortO)?shortO+1:0)];
				outOR = aOR[countO-((countO > shortO)?shortO+1:0)];
			} else {
				outOL = (outKL - (outIL + outJL + outLL)) - (aOL[(countO+1)-((countO+1 > shortO)?shortO+1:0)]*0.618033988749894848204586);
				aOL[countO] = outOL; outOL *= 0.618033988749894848204586;
				outOR = (outKR - (outIR + outJR + outLR)) - (aOR[(countO+1)-((countO+1 > shortO)?shortO+1:0)]*0.618033988749894848204586);
				aOR[countO] = outOR; outOR *= 0.618033988749894848204586;
				countO++; if (countO < 0 || countO > shortO) countO = 0;
				outOL += aOL[countO-((countO > shortO)?shortO+1:0)];
				outOR += aOR[countO-((countO > shortO)?shortO+1:0)];
			}
			
			if (diffuse < 9) {
				aPL[countP] = outLL - (outIL + outJL + outKL);
				aPR[countP] = outLR - (outIR + outJR + outKR);
				countP++; if (countP < 0 || countP > shortP) countP = 0;
				outPL = aPL[countP-((countP > shortP)?shortP+1:0)];
				outPR = aPR[countP-((countP > shortP)?shortP+1:0)];
			} else {
				outPL = (outLL - (outIL + outJL + outKL)) - (aPL[(countP+1)-((countP+1 > shortP)?shortP+1:0)]*0.618033988749894848204586);
				aPL[countP] = outPL; outPL *= 0.618033988749894848204586;
				outPR = (outLR - (outIR + outJR + outKR)) - (aPR[(countP+1)-((countP+1 > shortP)?shortP+1:0)]*0.618033988749894848204586);
				aPR[countP] = outPR; outPR *= 0.618033988749894848204586;
				countP++; if (countP < 0 || countP > shortP) countP = 0;
				outPL += aPL[countP-((countP > shortP)?shortP+1:0)];
				outPR += aPR[countP-((countP > shortP)?shortP+1:0)];
			}
			
			double steerAL = outML - (outNL + outOL + outPL);
			double steerBL = outNL - (outML + outOL + outPL);
			double steerCL = outOL - (outML + outNL + outPL);
			double steerDL = outPL - (outML + outNL + outOL);
			
			double steerAR = outMR - (outNR + outOR + outPR);
			double steerBR = outNR - (outMR + outOR + outPR);
			double steerCR = outOR - (outMR + outNR + outPR);
			double steerDR = outPR - (outMR + outNR + outOR);
			
			feedbackAL = (steerAL * (1.0-crossfade)) + (steerAR * crossfade);
			feedbackBL = (steerBL * (1.0-crossfade)) + (steerBR * crossfade);
			feedbackCL = (steerCL * (1.0-crossfade)) + (steerCR * crossfade);
			feedbackDL = (steerDL * (1.0-crossfade)) + (steerDR * crossfade);
			
			feedbackAR = (steerAR * (1.0-crossfade)) + (steerAL * crossfade);
			feedbackBR = (steerBR * (1.0-crossfade)) + (steerBL * crossfade);
			feedbackCR = (steerCR * (1.0-crossfade)) + (steerCL * crossfade);
			feedbackDR = (steerDR * (1.0-crossfade)) + (steerDL * crossfade);
			//which we need to feed back into the input again, a bit
			
			inputSampleL = (outML + outNL + outOL + outPL)/16.0;
			inputSampleR = (outMR + outNR + outOR + outPR)/16.0;
			//and take the final combined sum of outputs, corrected for Householder gain and averaging
			
			bez[bez_CL] = bez[bez_BL];
			bez[bez_BL] = bez[bez_AL];
			bez[bez_AL] = inputSampleL;
			bez[bez_SampL] = 0.0;
			
			bez[bez_CR] = bez[bez_BR];
			bez[bez_BR] = bez[bez_AR];
			bez[bez_AR] = inputSampleR;
			bez[bez_SampR] = 0.0;
		}
		double CBL = (bez[bez_CL]*(1.0-bez[bez_cycle]))+(bez[bez_BL]*bez[bez_cycle]);
		double CBR = (bez[bez_CR]*(1.0-bez[bez_cycle]))+(bez[bez_BR]*bez[bez_cycle]);
		double BAL = (bez[bez_BL]*(1.0-bez[bez_cycle]))+(bez[bez_AL]*bez[bez_cycle]);
		double BAR = (bez[bez_BR]*(1.0-bez[bez_cycle]))+(bez[bez_AR]*bez[bez_cycle]);
		double CBAL = (bez[bez_BL]+(CBL*(1.0-bez[bez_cycle]))+(BAL*bez[bez_cycle]))*0.125;
		double CBAR = (bez[bez_BR]+(CBR*(1.0-bez[bez_cycle]))+(BAR*bez[bez_cycle]))*0.125;
		inputSampleL = CBAL;
		inputSampleR = CBAR;
		
		iirOutL = (inputSampleL*iirExt)+(iirOutL*(1.0-iirExt)); inputSampleL = iirOutL;
		iirOutR = (inputSampleR*iirExt)+(iirOutR*(1.0-iirExt)); inputSampleR = iirOutR;
		
		inputSampleL = (inputSampleL*wet)+(drySampleL*(1.0-wet));
		inputSampleR = (inputSampleR*wet)+(drySampleR*(1.0-wet));
		
		if (inputSampleL > 1.0) inputSampleL = 1.0;
		if (inputSampleL < -1.0) inputSampleL = -1.0;
		if (inputSampleR > 1.0) inputSampleR = 1.0;
		if (inputSampleR < -1.0) inputSampleR = -1.0;
		
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
	for(int count = 0; count < kshortA+2; count++) {aAL[count] = 0.0; aAR[count] = 0.0;}
	for(int count = 0; count < kshortB+2; count++) {aBL[count] = 0.0; aBR[count] = 0.0;}
	for(int count = 0; count < kshortC+2; count++) {aCL[count] = 0.0; aCR[count] = 0.0;}
	for(int count = 0; count < kshortD+2; count++) {aDL[count] = 0.0; aDR[count] = 0.0;}
	for(int count = 0; count < kshortE+2; count++) {aEL[count] = 0.0; aER[count] = 0.0;}
	for(int count = 0; count < kshortF+2; count++) {aFL[count] = 0.0; aFR[count] = 0.0;}
	for(int count = 0; count < kshortG+2; count++) {aGL[count] = 0.0; aGR[count] = 0.0;}
	for(int count = 0; count < kshortH+2; count++) {aHL[count] = 0.0; aHR[count] = 0.0;}
	for(int count = 0; count < kshortI+2; count++) {aIL[count] = 0.0; aIR[count] = 0.0;}
	for(int count = 0; count < kshortJ+2; count++) {aJL[count] = 0.0; aJR[count] = 0.0;}
	for(int count = 0; count < kshortK+2; count++) {aKL[count] = 0.0; aKR[count] = 0.0;}
	for(int count = 0; count < kshortL+2; count++) {aLL[count] = 0.0; aLR[count] = 0.0;}
	for(int count = 0; count < kshortM+2; count++) {aML[count] = 0.0; aMR[count] = 0.0;}
	for(int count = 0; count < kshortN+2; count++) {aNL[count] = 0.0; aNR[count] = 0.0;}
	for(int count = 0; count < kshortO+2; count++) {aOL[count] = 0.0; aOR[count] = 0.0;}
	for(int count = 0; count < kshortP+2; count++) {aPL[count] = 0.0; aPR[count] = 0.0;}
	
	feedbackAL = 0.0;
	feedbackBL = 0.0;
	feedbackCL = 0.0;
	feedbackDL = 0.0;
	feedbackAR = 0.0;
	feedbackBR = 0.0;
	feedbackCR = 0.0;
	feedbackDR = 0.0;
	
	iirInL = 0.0;
	iirFAL = 0.0;
	iirFBL = 0.0;
	iirFCL = 0.0;
	iirFDL = 0.0;
	iirOutL = 0.0;
	iirInR = 0.0;
	iirFAR = 0.0;
	iirFBR = 0.0;
	iirFCR = 0.0;
	iirFDR = 0.0;
	iirOutR = 0.0;
	
	countA = 1;
	countB = 1;
	countC = 1;
	countD = 1;	
	countE = 1;
	countF = 1;
	countG = 1;
	countH = 1;
	countI = 1;
	countJ = 1;
	countK = 1;
	countL = 1;
	countM = 1;
	countN = 1;
	countO = 1;
	countP = 1;
	
	shortA = 192;  shortB = 463;  shortC = 420;  shortD = 34;  shortE = 161;  
	shortF = 426;  shortG = 160;  shortH = 474;  shortI = 21;  shortJ = 343;  
	shortK = 296;  shortL = 294;  shortM = 11;  shortN = 987;  shortO = 64;  
	shortP = 482; //5 to 51 ms, 94 seat room  
	
	for (int x = 0; x < bez_total; x++) bez[x] = 0.0;
	bez[bez_cycle] = 1.0;
	
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	return noErr;
}

};

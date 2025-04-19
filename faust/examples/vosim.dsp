import("stdfaust.lib");
tablesize = 2048;
hump = rdtable(tablesize, os.coswaveform(tablesize)) : -(1) : /(-2);
//gaussish_w(period, w) = ba.spulse(width - inout) : en.asr(inout, tablesize / 2, inout) : hump
//with {
//    width = ma.SR * period * w;
//    inout = (period * (1 - w)) / 100;
//};
gaussish_t(period, w) = ba.spulse(width - inout - 1) : en.asr(inout, tablesize / 2, inout) : hump
with {
    width = ma.SR * w;
    inout = (period - w) / 100;
};
flavor(freq, reset) = os.hs_phasor(tablesize, freq, reset) : hump;
pulsar(nPulses, fund, form) = ba.pulsen(1, samples) <: flavor(form) * gaussish_t(period, nPulses : /(form))
with {
    period = 1/fund;
    samples = ma.SR/fund;
};
postprocess = fi.lowpass(2, ma.SR / 2.1) : fi.dcblocker;
asFreq(low, v) = low * (2 : pow(v));
process = pulsar(pulses : rint, fundamental : ba.semi2ratio : *(440), formant : scaleToF) : *(ba.lin2LogGain(vca)) : postprocess <: _, _; 

scaleToF = asFreq(440);
fundamental = hslider("[00]fundamental", -24, -60, 60, 0.1);
formant = hslider("[01]formant", 1, -5, 5, 0.001);
vca = hslider("[02]vca", 0.5, 0, 1, .001);
pulses = hslider("[03]N", 3, 1, 20, 1);

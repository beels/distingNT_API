#include <math.h>
#include <new>
#include <distingnt/api.h>

// Algorithm state
struct _midiCcConverterAlgorithm : public _NT_algorithm
{
    _midiCcConverterAlgorithm()
    {
        msb_value = 0;
        lsb_value = 0;
        msb_received = false;
        lsb_received = false;
        current_voltage = 0.0f;
        smoothing_coef = 1.0f;
    }

    // MIDI state
    int msb_value;     // Most significant byte value (0-127)
    int lsb_value;     // Least significant byte value (0-127)
    bool msb_received; // Flag to track if we've received an MSB message
    bool lsb_received; // Flag to track if we've received an LSB message

    // Output state
    float current_voltage; // Current output voltage
    float smoothing_coef;  // Cached smoothing coefficient
};

// Parameter indices
enum
{
    kParamOutput,      // CV output
    kParamOutputMode,  // CV output mode (add/replace)
    kParamCC,          // CC number
    kParamMidiChannel, // MIDI channel (0 = all, 1-16 = specific)
    kParamBipolar,     // Bipolar mode (on/off)
    kParamSmoothing,   // Smoothing amount
};

// clang-format off
// Parameter definitions
_NT_parameter parameters[] = {
    NT_PARAMETER_CV_OUTPUT_WITH_MODE("CV Output", 1, 13)
    {.name = "CC", .min = 0, .max = 127, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL},
    {.name = "MIDI Ch", .min = 0, .max = 16, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL},
    {.name = "Bipolar", .min = 0, .max = 1, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL},
    {.name = "Smoothing", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
};
// clang-format on

// Parameter pages
uint8_t page1[] = {kParamCC, kParamMidiChannel};
uint8_t page2[] = {kParamBipolar, kParamSmoothing};
uint8_t page3[] = {kParamOutput, kParamOutputMode};

_NT_parameterPage pages[] = {
    {.name = "Setup", .numParams = ARRAY_SIZE(page1), .params = page1},
    {.name = "CV", .numParams = ARRAY_SIZE(page2), .params = page2},
    {.name = "Routing", .numParams = ARRAY_SIZE(page3), .params = page3},
};

_NT_parameterPages parameterPages = {
    .numPages = ARRAY_SIZE(pages),
    .pages = pages,
};

// Calculate memory requirements for the algorithm
void calculateRequirements(_NT_algorithmRequirements &req)
{
    req.numParameters = ARRAY_SIZE(parameters);
    req.sram = sizeof(_midiCcConverterAlgorithm);
    req.dram = 0;
    req.dtc = 0;
    req.itc = 0;
}

// Construct the algorithm
_NT_algorithm *construct(const _NT_algorithmMemoryPtrs &ptrs, const _NT_algorithmRequirements &req)
{
    _midiCcConverterAlgorithm *alg = new (ptrs.sram) _midiCcConverterAlgorithm();
    alg->parameters = parameters;
    alg->parameterPages = &parameterPages;
    return alg;
}

// Process incoming MIDI messages
void midiMessage(_NT_algorithm *self, uint8_t byte0, uint8_t byte1, uint8_t byte2)
{
    _midiCcConverterAlgorithm *pThis = (_midiCcConverterAlgorithm *)self;

    // Only process CC messages (0xB0-0xBF)
    if ((byte0 & 0xF0) == 0xB0)
    {
        // Extract MIDI channel (0-15)
        int channel = byte0 & 0x0F;

        // Get the MIDI channel parameter (0 = all channels, 1-16 = specific channel)
        int midiChannel = pThis->v[kParamMidiChannel];

        // Skip if we're filtering by channel and this isn't our channel
        if (midiChannel > 0 && channel != (midiChannel - 1))
        {
            return;
        }

        // Extract CC number and value
        int cc_number = byte1;
        int cc_value = byte2;

        // Get MSB from parameters, LSB is always MSB + 32
        int msb_cc = pThis->v[kParamCC];
        int lsb_cc = msb_cc + 32;

        // Check for reset controllers message (CC 121) or all notes off (CC 123)
        if (cc_number == 121 || (cc_number == 123 && cc_value == 0))
        {
            pThis->msb_value = 0;
            pThis->lsb_value = 0;
            pThis->msb_received = false;
            pThis->lsb_received = false;
            pThis->current_voltage = 0.0f;
            return;
        }

        // Check if this is our MSB or LSB CC number
        if (cc_number == msb_cc)
        {
            pThis->msb_value = cc_value;
            pThis->msb_received = true;
        }
        else if (cc_number == lsb_cc)
        {
            pThis->lsb_value = cc_value;
            pThis->lsb_received = true;
        }

        // Calculate the voltage if we have received any CC messages
        if (pThis->msb_received || pThis->lsb_received)
        {
            // Calculate 14-bit value (0-16383)
            int value_14bit = 0;

            if (pThis->msb_received && pThis->lsb_received)
            {
                // We have both MSB and LSB - ideal case
                value_14bit = (pThis->msb_value << 7) | pThis->lsb_value;
            }
            else if (pThis->msb_received)
            {
                // We only have MSB, scale it up
                value_14bit = pThis->msb_value << 7;
            }
            else
            {
                // We only have LSB, just use it at low resolution
                value_14bit = pThis->lsb_value;
            }

            // Normalize to 0.0-1.0 range
            float normalized = (float)value_14bit / 16383.0f;

            // Apply bipolar setting if enabled
            if (pThis->v[kParamBipolar])
            {
                // -5V to +5V range
                pThis->current_voltage = (normalized * 10.0f) - 5.0f;
            }
            else
            {
                // 0V to +10V range
                pThis->current_voltage = normalized * 10.0f;
            }
        }
    }
}

// Add parameter changed handler
void parameterChanged(_NT_algorithm *self, int p)
{
    _midiCcConverterAlgorithm *pThis = (_midiCcConverterAlgorithm *)self;

    if (p == kParamSmoothing)
    {
        // Get smoothing value (0-100%)
        float smoothing = pThis->v[kParamSmoothing] / 100.0f;

        // Calculate smoothing coefficient using exponential approach
        if (smoothing > 0.0f)
        {
            // Convert percentage to time constant in milliseconds (1% = 1ms, 100% = 500ms)
            float timeConstantMs = smoothing * 500.0f;
            if (timeConstantMs < 1.0f)
                timeConstantMs = 1.0f;

            // Time constant formula: alpha = exp(-T/tau)
            // For a simple single-frame approach, T is 1/sampleRate
            float sampleRate = (float)NT_globals.sampleRate;
            float timeConstantSamples = (timeConstantMs / 1000.0f) * sampleRate;

            // This gives us a coefficient that determines how quickly we approach the target
            pThis->smoothing_coef = 1.0f - expf(-1.0f / timeConstantSamples);
        }
        else
        {
            pThis->smoothing_coef = 1.0f; // No smoothing
        }
    }
}

// DSP processing function
void step(_NT_algorithm *self, float *busFrames, int numFramesBy4)
{
    _midiCcConverterAlgorithm *pThis = (_midiCcConverterAlgorithm *)self;
    int numFrames = numFramesBy4 * 4;

    // Get output destinations with bounds checking
    int cvOutputIdx = pThis->v[kParamOutput] - 1;
    if (cvOutputIdx < 0)
    {
        return; // Invalid output
    }

    float *cvOutput = busFrames + cvOutputIdx * numFrames;
    bool cvReplace = pThis->v[kParamOutputMode];

    // Apply CV to output bus with exponential smoothing
    float target_voltage = pThis->current_voltage;
    float current_voltage = pThis->current_voltage;

    for (int i = 0; i < numFrames; ++i)
    {
        // One-pole lowpass filter: y[n] = y[n-1] + α(x[n] - y[n-1])
        current_voltage = current_voltage + (pThis->smoothing_coef * (target_voltage - current_voltage));

        // Output to CV
        if (cvReplace)
        {
            cvOutput[i] = current_voltage;
        }
        else
        {
            cvOutput[i] += current_voltage;
        }
    }

    // Save the last voltage for next time (for smoothing)
    pThis->current_voltage = current_voltage;
}

// Factory information
_NT_factory factory = {
    .guid = NT_MULTICHAR('T', 'h', '1', '4'),
    .name = "14-bit CC to CV",
    .description = "Converts 14-bit MIDI CC messages to CV",
    .calculateRequirements = calculateRequirements,
    .construct = construct,
    .parameterChanged = parameterChanged,
    .step = step,
    .draw = NULL,
    .midiMessage = midiMessage,
};

// Plugin entry point
uintptr_t pluginEntry(_NT_selector selector, uint32_t data)
{
    switch (selector)
    {
    case kNT_selector_version:
        return kNT_apiVersionCurrent;
    case kNT_selector_numFactories:
        return 1;
    case kNT_selector_factoryInfo:
        return (uintptr_t)((data == 0) ? &factory : NULL);
    }
    return 0;
}

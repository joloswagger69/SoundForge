#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <iterator>

// Reihenfolge muss exakt der Tabelle "defs" entsprechen.
namespace P
{
enum Id
{
    osc1Wave, osc1Oct, osc1Level,
    osc2Wave, osc2Oct, osc2Detune, osc2Level,
    subLevel, noiseLevel,
    uniVoices, uniDetune, uniSpread,
    filtType, cutoff, reso, filtEnv,
    ampA, ampD, ampS, ampR,
    fltA, fltD, fltS, fltR,
    lfoRate, lfoCut, lfoPitch,
    drive, chorus, delayMix, delayTime, delayFb, reverbMix, reverbSize,
    master,
    kCount
};
}

struct ParamDef
{
    const char* id;
    const char* name;
    char kind; // 'F' float, 'I' int, 'C' choice
    float lo, hi, def, skew;
    const char* choices;
};

inline constexpr ParamDef defs[] = {
    { "osc1Wave",   "Wave",       'C', 0, 3, 0, 1, "Saw|Square|Sine|Triangle" },
    { "osc1Oct",    "Octave",     'I', -2, 2, 0, 1, "" },
    { "osc1Level",  "Level",      'F', 0, 1, 0.8f, 1, "" },

    { "osc2Wave",   "Wave",       'C', 0, 3, 0, 1, "Saw|Square|Sine|Triangle" },
    { "osc2Oct",    "Octave",     'I', -2, 2, 0, 1, "" },
    { "osc2Detune", "Detune",     'F', -50, 50, 7, 1, "" },
    { "osc2Level",  "Level",      'F', 0, 1, 0.6f, 1, "" },

    { "subLevel",   "Sub",        'F', 0, 1, 0, 1, "" },
    { "noiseLevel", "Noise",      'F', 0, 1, 0, 1, "" },

    { "uniVoices",  "Voices",     'I', 1, 7, 1, 1, "" },
    { "uniDetune",  "Detune",     'F', 0, 100, 15, 1, "" },
    { "uniSpread",  "Spread",     'F', 0, 1, 0.6f, 1, "" },

    { "filtType",   "Type",       'C', 0, 2, 0, 1, "Lowpass|Highpass|Bandpass" },
    { "cutoff",     "Cutoff",     'F', 20, 20000, 8000, 0.3f, "" },
    { "reso",       "Resonance",  'F', 0, 1, 0.2f, 1, "" },
    { "filtEnv",    "Env Amt",    'F', -1, 1, 0.3f, 1, "" },

    { "ampA",       "Attack",     'F', 0.001f, 5, 0.005f, 0.4f, "" },
    { "ampD",       "Decay",      'F', 0.001f, 5, 0.3f, 0.4f, "" },
    { "ampS",       "Sustain",    'F', 0, 1, 0.7f, 1, "" },
    { "ampR",       "Release",    'F', 0.001f, 8, 0.25f, 0.4f, "" },

    { "fltA",       "Attack",     'F', 0.001f, 5, 0.005f, 0.4f, "" },
    { "fltD",       "Decay",      'F', 0.001f, 5, 0.4f, 0.4f, "" },
    { "fltS",       "Sustain",    'F', 0, 1, 0.3f, 1, "" },
    { "fltR",       "Release",    'F', 0.001f, 8, 0.3f, 0.4f, "" },

    { "lfoRate",    "Rate",       'F', 0.05f, 20, 2, 0.5f, "" },
    { "lfoCut",     "> Cutoff",   'F', 0, 1, 0, 1, "" },
    { "lfoPitch",   "> Pitch",    'F', 0, 1, 0, 1, "" },

    { "drive",      "Drive",      'F', 0, 1, 0, 1, "" },
    { "chorus",     "Chorus",     'F', 0, 1, 0, 1, "" },
    { "delayMix",   "Delay",      'F', 0, 1, 0, 1, "" },
    { "delayTime",  "Time",       'F', 0.05f, 1, 0.35f, 1, "" },
    { "delayFb",    "Feedback",   'F', 0, 0.9f, 0.35f, 1, "" },
    { "reverbMix",  "Reverb",     'F', 0, 1, 0.1f, 1, "" },
    { "reverbSize", "Size",       'F', 0, 1, 0.5f, 1, "" },

    { "master",     "Volume",     'F', 0, 1, 0.7f, 1, "" },
};
static_assert(std::size(defs) == (size_t) P::kCount, "defs und enum P::Id passen nicht zusammen");

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (auto& d : defs)
    {
        juce::ParameterID pid { d.id, 1 };
        if (d.kind == 'C')
            layout.add(std::make_unique<juce::AudioParameterChoice>(
                pid, d.name, juce::StringArray::fromTokens(d.choices, "|", ""), (int) d.def));
        else if (d.kind == 'I')
            layout.add(std::make_unique<juce::AudioParameterInt>(
                pid, d.name, (int) d.lo, (int) d.hi, (int) d.def));
        else
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                pid, d.name, juce::NormalisableRange<float>(d.lo, d.hi, 0.f, d.skew), d.def));
    }
    return layout;
}

// Schneller Zugriff auf die Rohwerte (audio-thread-sicher)
struct ParamCache
{
    std::array<std::atomic<float>*, P::kCount> a {};
    float operator[](int i) const noexcept { return a[(size_t) i]->load(); }
};

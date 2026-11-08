#pragma once
#include "Params.h"
#include <vector>

// Alle nicht genannten Parameter werden beim Laden auf ihren Standardwert gesetzt.
struct Preset
{
    const char* name;
    std::vector<std::pair<int, float>> v;
};

inline const std::vector<Preset>& factoryPresets()
{
    using namespace P;
    static const std::vector<Preset> list = {
        { "Init", {} },

        { "Supersaw Lead", {
            { osc2Detune, 12 }, { uniVoices, 7 }, { uniDetune, 28 }, { uniSpread, 0.9f },
            { cutoff, 9000 }, { reso, 0.15f }, { ampS, 0.8f }, { ampR, 0.35f },
            { chorus, 0.3f }, { delayMix, 0.15f }, { reverbMix, 0.2f } } },

        { "Deep Sub Bass", {
            { osc1Wave, 2 }, { osc2Oct, -1 }, { osc2Level, 0.3f }, { subLevel, 0.7f },
            { cutoff, 600 }, { filtEnv, 0.2f }, { reso, 0.1f }, { ampA, 0.003f },
            { ampS, 1.f }, { ampR, 0.12f }, { drive, 0.25f }, { reverbMix, 0.f } } },

        { "Reese Bass", {
            { osc1Oct, -1 }, { osc2Oct, -1 }, { osc2Detune, 18 }, { uniVoices, 3 }, { uniDetune, 14 },
            { cutoff, 900 }, { reso, 0.35f }, { filtEnv, 0.3f }, { lfoRate, 0.4f }, { lfoCut, 0.25f },
            { drive, 0.3f }, { ampS, 1.f }, { ampR, 0.2f }, { reverbMix, 0.f } } },

        { "Pluck Trance", {
            { osc2Detune, 10 }, { uniVoices, 3 }, { uniDetune, 14 }, { cutoff, 1500 }, { filtEnv, 0.8f },
            { fltA, 0.001f }, { fltD, 0.22f }, { fltS, 0.f }, { ampD, 0.25f }, { ampS, 0.f }, { ampR, 0.2f },
            { delayMix, 0.25f }, { delayTime, 0.375f }, { delayFb, 0.4f }, { reverbMix, 0.2f } } },

        { "Warm Pad", {
            { osc2Detune, 9 }, { uniVoices, 5 }, { uniDetune, 20 }, { uniSpread, 1.f },
            { cutoff, 2200 }, { reso, 0.1f }, { filtEnv, 0.3f }, { fltA, 1.5f },
            { ampA, 1.2f }, { ampD, 1.f }, { ampS, 0.8f }, { ampR, 2.5f },
            { lfoRate, 0.25f }, { lfoCut, 0.1f }, { chorus, 0.45f }, { reverbMix, 0.45f }, { reverbSize, 0.8f } } },

        { "Glass Bells", {
            { osc1Wave, 2 }, { osc2Wave, 2 }, { osc2Oct, 2 }, { osc2Level, 0.4f }, { osc2Detune, 6 },
            { cutoff, 12000 }, { filtEnv, 0.f }, { ampA, 0.002f }, { ampD, 2.f }, { ampS, 0.f }, { ampR, 1.5f },
            { chorus, 0.2f }, { delayMix, 0.2f }, { reverbMix, 0.5f }, { reverbSize, 0.8f } } },

        { "Acid 303", {
            { osc2Level, 0.f }, { cutoff, 600 }, { reso, 0.8f }, { filtEnv, 0.75f },
            { fltD, 0.18f }, { fltS, 0.f }, { ampS, 0.8f }, { ampR, 0.08f }, { drive, 0.4f }, { reverbMix, 0.f } } },

        { "Hard Square Lead", {
            { osc1Wave, 1 }, { osc2Wave, 1 }, { osc2Oct, 1 }, { osc2Level, 0.3f }, { uniVoices, 3 }, { uniDetune, 10 },
            { cutoff, 5000 }, { reso, 0.25f }, { ampS, 0.9f }, { delayMix, 0.25f }, { reverbMix, 0.15f } } },

        { "EDM Pluck Chord", {
            { uniVoices, 5 }, { uniDetune, 18 }, { uniSpread, 0.8f }, { cutoff, 2500 }, { filtEnv, 0.6f },
            { fltD, 0.3f }, { fltS, 0.1f }, { ampD, 0.6f }, { ampS, 0.15f }, { ampR, 0.3f },
            { chorus, 0.25f }, { reverbMix, 0.25f } } },

        { "Dreamy Keys", {
            { osc1Wave, 3 }, { osc2Wave, 2 }, { osc2Oct, 1 }, { osc2Level, 0.3f }, { cutoff, 5000 }, { filtEnv, 0.2f },
            { ampD, 1.2f }, { ampS, 0.3f }, { ampR, 0.8f }, { chorus, 0.4f }, { reverbMix, 0.35f } } },

        { "Wobble Bass", {
            { osc2Wave, 1 }, { osc2Oct, -1 }, { osc2Level, 0.5f }, { cutoff, 500 }, { reso, 0.55f }, { filtEnv, 0.f },
            { lfoRate, 3.f }, { lfoCut, 0.9f }, { drive, 0.35f }, { ampS, 1.f }, { ampR, 0.15f }, { reverbMix, 0.f } } },

        { "Cinematic Drone", {
            { osc2Oct, -1 }, { uniVoices, 7 }, { uniDetune, 40 }, { uniSpread, 1.f }, { cutoff, 700 },
            { filtEnv, 0.f }, { lfoRate, 0.1f }, { lfoCut, 0.3f }, { ampA, 3.f }, { ampS, 1.f }, { ampR, 5.f },
            { chorus, 0.3f }, { reverbMix, 0.7f }, { reverbSize, 0.95f } } },

        { "Future Bass Chord", {
            { osc2Detune, 15 }, { uniVoices, 7 }, { uniDetune, 35 }, { uniSpread, 1.f }, { cutoff, 3500 },
            { filtEnv, 0.4f }, { fltD, 0.5f }, { fltS, 0.4f }, { ampA, 0.01f }, { ampS, 0.9f },
            { drive, 0.1f }, { chorus, 0.5f }, { delayMix, 0.2f }, { reverbMix, 0.3f } } },

        { "Retro Arp", {
            { osc1Wave, 1 }, { osc2Detune, 6 }, { cutoff, 3000 }, { filtEnv, 0.5f }, { fltD, 0.15f }, { fltS, 0.2f },
            { ampD, 0.2f }, { ampS, 0.3f }, { ampR, 0.1f }, { delayMix, 0.3f }, { delayTime, 0.3f },
            { delayFb, 0.45f }, { reverbMix, 0.15f } } },

        { "Soft Sine Lead", {
            { osc1Wave, 2 }, { osc2Wave, 3 }, { osc2Oct, 1 }, { osc2Level, 0.2f }, { cutoff, 12000 }, { filtEnv, 0.f },
            { ampA, 0.02f }, { ampS, 0.9f }, { ampR, 0.3f }, { lfoRate, 5.5f }, { lfoPitch, 0.07f },
            { delayMix, 0.2f }, { reverbMix, 0.25f } } },

        { "Dark Techno Stab", {
            { osc2Oct, -1 }, { cutoff, 800 }, { reso, 0.4f }, { filtEnv, 0.7f }, { fltD, 0.18f }, { fltS, 0.1f },
            { ampD, 0.2f }, { ampS, 0.f }, { ampR, 0.15f }, { drive, 0.2f },
            { delayMix, 0.3f }, { delayTime, 0.25f }, { delayFb, 0.4f }, { reverbMix, 0.2f } } },

        { "Noise Riser FX", {
            { osc1Level, 0.3f }, { osc2Level, 0.f }, { noiseLevel, 0.9f }, { cutoff, 500 }, { reso, 0.5f },
            { filtEnv, 1.f }, { fltA, 4.f }, { fltD, 0.1f }, { fltS, 1.f }, { ampA, 3.f }, { ampS, 1.f },
            { ampR, 1.5f }, { reverbMix, 0.4f } } },
    };
    return list;
}

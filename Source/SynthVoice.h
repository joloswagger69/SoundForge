#pragma once
#include "Params.h"

class SFSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

namespace sfdsp
{
inline float polyBlep(float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.f;
    }
    if (t > 1.f - dt)
    {
        t = (t - 1.f) / dt;
        return t * t + t + t + 1.f;
    }
    return 0.f;
}

inline float osc(int shape, float ph, float dt)
{
    switch (shape)
    {
        case 0: return 2.f * ph - 1.f - polyBlep(ph, dt);
        case 1:
        {
            float v = ph < 0.5f ? 1.f : -1.f;
            v += polyBlep(ph, dt);
            float t2 = ph + 0.5f;
            if (t2 >= 1.f) t2 -= 1.f;
            v -= polyBlep(t2, dt);
            return v * 0.8f;
        }
        case 2: return std::sin(juce::MathConstants<float>::twoPi * ph);
        default: return 4.f * std::abs(ph - 0.5f) - 1.f;
    }
}
} // namespace sfdsp

class SFVoice : public juce::SynthesiserVoice
{
public:
    explicit SFVoice(const ParamCache& pc) : p(pc) {}

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<SFSound*>(s) != nullptr; }

    void setCurrentPlaybackSampleRate(double newRate) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
        if (newRate <= 0.0) return;
        sr = newRate;
        ampEnv.setSampleRate(sr);
        fltEnv.setSampleRate(sr);
        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        for (auto& f : filt) f.prepare(spec);
    }

    void startNote(int midiNote, float vel, juce::SynthesiserSound*, int pitchWheel) override
    {
        note = midiNote;
        velocity = vel;
        bend = (float) (pitchWheel - 8192) / 8192.f * 2.f;
        for (int u = 0; u < 7; ++u)
        {
            ph1[u] = rng.nextFloat();
            ph2[u] = rng.nextFloat();
        }
        subPh = 0.f;
        lfoPh = 0.f;
        fltEnvVal = 0.f;
        for (auto& f : filt) f.reset();
        updateEnvParams();
        ampEnv.noteOn();
        fltEnv.noteOn();
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnv.noteOff();
            fltEnv.noteOff();
        }
        else
        {
            ampEnv.reset();
            fltEnv.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int v) override { bend = (float) (v - 8192) / 8192.f * 2.f; }
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (!isVoiceActive()) return;

        updateEnvParams();

        const int nUni = juce::jlimit(1, 7, (int) std::lround(p[P::uniVoices]));
        const float detune = p[P::uniDetune];
        const float spread = p[P::uniSpread];
        const int w1 = (int) std::lround(p[P::osc1Wave]);
        const int w2 = (int) std::lround(p[P::osc2Wave]);
        const float lv1 = p[P::osc1Level], lv2 = p[P::osc2Level];
        const float sub = p[P::subLevel], noise = p[P::noiseLevel];
        const float base1 = (float) note + 12.f * std::round(p[P::osc1Oct]);
        const float base2 = (float) note + 12.f * std::round(p[P::osc2Oct]) + p[P::osc2Detune] * 0.01f;
        const float cutoffBase = p[P::cutoff];
        const float q = 0.7f + p[P::reso] * 9.f;
        const float envAmt = p[P::filtEnv];
        const float lfoCut = p[P::lfoCut];
        const float lfoPitchDepth = p[P::lfoPitch] * 2.f;
        const float lfoInc = p[P::lfoRate] / (float) sr;
        const int type = (int) std::lround(p[P::filtType]);
        const auto ftype = type == 0 ? juce::dsp::StateVariableTPTFilterType::lowpass
                         : type == 1 ? juce::dsp::StateVariableTPTFilterType::highpass
                                     : juce::dsp::StateVariableTPTFilterType::bandpass;
        for (auto& f : filt) f.setType(ftype);

        float gl[7], gr[7], off[7], dt1[7] = {}, dt2[7] = {};
        const float g = 1.f / std::sqrt((float) nUni);
        for (int u = 0; u < nUni; ++u)
        {
            const float pos = nUni > 1 ? ((float) u / (float) (nUni - 1)) * 2.f - 1.f : 0.f;
            const float angle = (pos * spread + 1.f) * 0.25f * juce::MathConstants<float>::pi;
            gl[u] = std::cos(angle) * g;
            gr[u] = std::sin(angle) * g;
            off[u] = pos * detune;
        }

        auto semiToInc = [this](float s)
        {
            return juce::jmin(0.45f, 440.f * std::exp2((s - 69.f) / 12.f) / (float) sr);
        };

        float dtSub = 0.f, lfoV = 0.f;

        for (int i = 0; i < num; ++i)
        {
            if ((i & 15) == 0)
            {
                lfoV = std::sin(juce::MathConstants<float>::twoPi * lfoPh);
                lfoPh += lfoInc * 16.f;
                if (lfoPh >= 1.f) lfoPh -= 1.f;

                const float pm = lfoV * lfoPitchDepth + bend;
                for (int u = 0; u < nUni; ++u)
                {
                    dt1[u] = semiToInc(base1 + pm + off[u] * 0.01f);
                    dt2[u] = semiToInc(base2 + pm + off[u] * 0.01f);
                }
                dtSub = semiToInc((float) note - 12.f + pm);

                const float cut = juce::jlimit(20.f, (float) sr * 0.45f,
                    cutoffBase * std::exp2(envAmt * 7.f * fltEnvVal + lfoCut * lfoV * 2.f));
                for (auto& f : filt)
                {
                    f.setCutoffFrequency(cut);
                    f.setResonance(q);
                }
            }

            fltEnvVal = fltEnv.getNextSample();

            float l = 0.f, r = 0.f;
            for (int u = 0; u < nUni; ++u)
            {
                const float s = lv1 * sfdsp::osc(w1, ph1[u], dt1[u]) + lv2 * sfdsp::osc(w2, ph2[u], dt2[u]);
                ph1[u] += dt1[u]; if (ph1[u] >= 1.f) ph1[u] -= 1.f;
                ph2[u] += dt2[u]; if (ph2[u] >= 1.f) ph2[u] -= 1.f;
                l += s * gl[u];
                r += s * gr[u];
            }

            const float sb = sub * std::sin(juce::MathConstants<float>::twoPi * subPh);
            subPh += dtSub; if (subPh >= 1.f) subPh -= 1.f;
            const float nz = noise * (rng.nextFloat() * 2.f - 1.f) * 0.6f;
            l += sb + nz;
            r += sb + nz;

            l = filt[0].processSample(0, l);
            r = filt[1].processSample(0, r);

            float a = ampEnv.getNextSample();
            if (!ampEnv.isActive())
            {
                clearCurrentNote();
                break;
            }
            a *= (0.3f + 0.7f * velocity) * 0.35f;

            out.addSample(0, start + i, l * a);
            if (out.getNumChannels() > 1)
                out.addSample(1, start + i, r * a);
        }
    }

private:
    void updateEnvParams()
    {
        ampEnv.setParameters({ p[P::ampA], p[P::ampD], p[P::ampS], p[P::ampR] });
        fltEnv.setParameters({ p[P::fltA], p[P::fltD], p[P::fltS], p[P::fltR] });
    }

    const ParamCache& p;
    double sr = 44100.0;
    int note = 60;
    float velocity = 1.f, bend = 0.f;
    float ph1[7] = {}, ph2[7] = {}, subPh = 0.f, lfoPh = 0.f, fltEnvVal = 0.f;
    juce::ADSR ampEnv, fltEnv;
    juce::dsp::StateVariableTPTFilter<float> filt[2];
    juce::Random rng;
};

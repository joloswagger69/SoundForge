#include "PluginProcessor.h"
#include "PluginEditor.h"

SFProcessor::SFProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createLayout())
{
    for (int i = 0; i < P::kCount; ++i)
        cache.a[(size_t) i] = apvts.getRawParameterValue(defs[i].id);

    synth.addSound(new SFSound());
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SFVoice(cache));

    setCurrentProgram(currentProgram);
}

void SFProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    chorusFx.prepare(spec);
    chorusFx.reset();
    chorusFx.setRate(0.8f);
    chorusFx.setDepth(0.35f);
    chorusFx.setCentreDelay(7.f);
    chorusFx.setFeedback(0.f);

    delayLine.setMaximumDelayInSamples((int) (sampleRate * 1.2));
    delayLine.prepare(spec);
    delayLine.reset();
    delayState[0] = delayState[1] = 0.f;

    reverbFx.setSampleRate(sampleRate);
    reverbFx.reset();
}

bool SFProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SFProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, n);

    if (buffer.getNumChannels() < 2) return;

    // --- Drive ---
    const float drive = cache[P::drive];
    if (drive > 0.001f)
    {
        const float k = 1.f + drive * 12.f;
        const float comp = 1.f / (1.f + 2.f * drive);
        for (int ch = 0; ch < 2; ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < n; ++i)
                d[i] = std::tanh(k * d[i]) * comp;
        }
    }

    // --- Chorus ---
    const float chorusMix = cache[P::chorus];
    if (chorusMix > 0.001f)
    {
        chorusFx.setMix(chorusMix * 0.7f);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        chorusFx.process(ctx);
    }

    // --- Delay ---
    const float dMix = cache[P::delayMix];
    if (dMix > 0.001f)
    {
        const float fb = cache[P::delayFb];
        delayLine.setDelay(juce::jmax(1.f, cache[P::delayTime] * (float) getSampleRate()));
        for (int ch = 0; ch < 2; ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < n; ++i)
            {
                delayLine.pushSample(ch, d[i] + delayState[ch] * fb);
                const float wet = delayLine.popSample(ch);
                delayState[ch] = wet;
                d[i] += wet * dMix;
            }
        }
    }

    // --- Reverb ---
    const float rMix = cache[P::reverbMix];
    if (rMix > 0.001f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.3f + cache[P::reverbSize] * 0.69f;
        rp.damping = 0.5f;
        rp.wetLevel = rMix * 0.4f;
        rp.dryLevel = 1.f - rMix * 0.25f;
        rp.width = 1.f;
        rp.freezeMode = 0.f;
        reverbFx.setParameters(rp);
        reverbFx.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), n);
    }

    // --- Master + Sicherheits-Limiter ---
    const float vol = cache[P::master] * 1.4f;
    for (int ch = 0; ch < 2; ++ch)
    {
        auto* d = buffer.getWritePointer(ch);
        for (int i = 0; i < n; ++i)
            d[i] = juce::jlimit(-1.f, 1.f, d[i] * vol);
    }
}

void SFProcessor::setCurrentProgram(int index)
{
    const auto& list = factoryPresets();
    if (index < 0 || index >= (int) list.size()) return;
    currentProgram = index;

    std::array<float, P::kCount> values;
    for (int i = 0; i < P::kCount; ++i) values[(size_t) i] = defs[i].def;
    for (auto& [id, v] : list[(size_t) index].v) values[(size_t) id] = v;

    for (int i = 0; i < P::kCount; ++i)
        if (auto* param = apvts.getParameter(defs[i].id))
            param->setValueNotifyingHost(param->convertTo0to1(values[(size_t) i]));
}

const juce::String SFProcessor::getProgramName(int index)
{
    const auto& list = factoryPresets();
    return (index >= 0 && index < (int) list.size()) ? juce::String(list[(size_t) index].name) : juce::String();
}

void SFProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void SFProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            currentProgram = juce::jlimit(0, getNumPrograms() - 1, (int) tree.getProperty("program", currentProgram));
            apvts.replaceState(tree);
        }
}

juce::AudioProcessorEditor* SFProcessor::createEditor() { return new SFEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SFProcessor(); }

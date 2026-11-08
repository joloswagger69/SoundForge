#pragma once
#include "PluginProcessor.h"

class SFEditor : public juce::AudioProcessorEditor
{
public:
    explicit SFEditor(SFProcessor&);
    ~SFEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider s { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };
    struct Choice
    {
        juce::ComboBox c;
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
    };
    struct Section
    {
        juce::String title;
        std::vector<int> items;
        juce::Rectangle<int> bounds;
    };

    SFProcessor& proc;
    juce::LookAndFeel_V4 laf;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };

    juce::OwnedArray<Knob> knobs;
    juce::OwnedArray<Choice> choices;
    std::array<Knob*, P::kCount> knobOf {};
    std::array<Choice*, P::kCount> choiceOf {};
    std::vector<std::vector<Section>> rows;

    void stepPreset(int delta);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SFEditor)
};

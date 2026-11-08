#include "PluginEditor.h"

static const juce::Colour kBg      { 0xff14161c };
static const juce::Colour kPanel   { 0xff1f232d };
static const juce::Colour kAccent  { 0xff00d1b2 };
static const juce::Colour kText    { 0xffe6e9ef };

SFEditor::SFEditor(SFProcessor& p) : juce::AudioProcessorEditor(&p), proc(p)
{
    laf.setColour(juce::Slider::rotarySliderFillColourId, kAccent);
    laf.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff3a4050));
    laf.setColour(juce::Slider::thumbColourId, kText);
    laf.setColour(juce::Slider::textBoxTextColourId, kText);
    laf.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    laf.setColour(juce::Label::textColourId, kText);
    laf.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2f3b));
    laf.setColour(juce::ComboBox::textColourId, kText);
    laf.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a4050));
    laf.setColour(juce::ComboBox::arrowColourId, kAccent);
    laf.setColour(juce::PopupMenu::backgroundColourId, kPanel);
    laf.setColour(juce::PopupMenu::textColourId, kText);
    laf.setColour(juce::PopupMenu::highlightedBackgroundColourId, kAccent.darker(0.4f));
    laf.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2f3b));
    laf.setColour(juce::TextButton::textColourOffId, kText);
    setLookAndFeel(&laf);

    using namespace P;
    rows = {
        { { "OSC 1",      { osc1Wave, osc1Oct, osc1Level }, {} },
          { "OSC 2",      { osc2Wave, osc2Oct, osc2Detune, osc2Level }, {} },
          { "MIX",        { subLevel, noiseLevel }, {} },
          { "UNISON",     { uniVoices, uniDetune, uniSpread }, {} } },
        { { "FILTER",     { filtType, cutoff, reso, filtEnv }, {} },
          { "AMP ENV",    { ampA, ampD, ampS, ampR }, {} },
          { "FILTER ENV", { fltA, fltD, fltS, fltR }, {} },
          { "LFO",        { lfoRate, lfoCut, lfoPitch }, {} } },
        { { "EFFECTS",    { drive, chorus, delayMix, delayTime, delayFb, reverbMix, reverbSize }, {} },
          { "OUTPUT",     { master }, {} } },
    };

    for (int i = 0; i < kCount; ++i)
    {
        const auto& d = defs[i];
        if (d.kind == 'C')
        {
            auto* c = choices.add(new Choice());
            const auto items = juce::StringArray::fromTokens(d.choices, "|", "");
            for (int k = 0; k < items.size(); ++k) c->c.addItem(items[k], k + 1);
            c->l.setText(d.name, juce::dontSendNotification);
            c->l.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(c->c);
            addAndMakeVisible(c->l);
            c->att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, d.id, c->c);
            choiceOf[(size_t) i] = c;
        }
        else
        {
            auto* k = knobs.add(new Knob());
            k->s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
            k->s.setDoubleClickReturnValue(true, d.def);
            k->l.setText(d.name, juce::dontSendNotification);
            k->l.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(k->s);
            addAndMakeVisible(k->l);
            k->att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, d.id, k->s);
            knobOf[(size_t) i] = k;
        }
    }

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem(proc.getProgramName(i), i + 1);
    presetBox.setSelectedItemIndex(proc.getCurrentProgram(), juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram(presetBox.getSelectedItemIndex()); };
    addAndMakeVisible(presetBox);

    prevBtn.onClick = [this] { stepPreset(-1); };
    nextBtn.onClick = [this] { stepPreset(1); };
    addAndMakeVisible(prevBtn);
    addAndMakeVisible(nextBtn);

    setSize(1100, 660);
}

SFEditor::~SFEditor() { setLookAndFeel(nullptr); }

void SFEditor::stepPreset(int delta)
{
    const int n = proc.getNumPrograms();
    const int idx = (proc.getCurrentProgram() + delta + n) % n;
    presetBox.setSelectedItemIndex(idx, juce::sendNotification);
}

void SFEditor::paint(juce::Graphics& g)
{
    g.fillAll(kBg);

    g.setColour(kAccent);
    g.setFont(juce::FontOptions(26.f, juce::Font::bold));
    g.drawText("SOUNDFORGE", 20, 12, 300, 36, juce::Justification::centredLeft);

    for (auto& row : rows)
        for (auto& s : row)
        {
            g.setColour(kPanel);
            g.fillRoundedRectangle(s.bounds.toFloat(), 8.f);
            g.setColour(kAccent.withAlpha(0.85f));
            g.setFont(juce::FontOptions(13.f, juce::Font::bold));
            g.drawText(s.title, s.bounds.getX() + 12, s.bounds.getY() + 6, s.bounds.getWidth() - 24, 18,
                       juce::Justification::centredLeft);
        }
}

void SFEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(60).reduced(16, 12);
    nextBtn.setBounds(header.removeFromRight(36));
    header.removeFromRight(6);
    presetBox.setBounds(header.removeFromRight(260));
    header.removeFromRight(6);
    prevBtn.setBounds(header.removeFromRight(36));

    area.reduce(12, 4);
    const int rowH = area.getHeight() / (int) rows.size();

    for (auto& row : rows)
    {
        auto rowArea = area.removeFromTop(rowH).reduced(0, 5);
        int total = 0;
        for (auto& s : row) total += (int) s.items.size();

        for (auto& s : row)
        {
            const int w = rowArea.getWidth() * (int) s.items.size() / juce::jmax(1, total);
            s.bounds = rowArea.removeFromLeft(w).reduced(4, 0);
            auto inner = s.bounds.reduced(6, 4);
            inner.removeFromTop(24);
            const int cw = inner.getWidth() / (int) s.items.size();

            for (int idx : s.items)
            {
                auto cell = inner.removeFromLeft(cw);
                if (auto* k = knobOf[(size_t) idx])
                {
                    k->l.setBounds(cell.removeFromTop(16));
                    k->s.setBounds(cell);
                }
                else if (auto* c = choiceOf[(size_t) idx])
                {
                    c->l.setBounds(cell.removeFromTop(16));
                    c->c.setBounds(cell.removeFromTop(28).reduced(4, 0).translated(0, 8));
                }
            }
        }
    }
    repaint();
}

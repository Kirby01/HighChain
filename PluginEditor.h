#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HighChainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit HighChainAudioProcessorEditor (HighChainAudioProcessor&);
    ~HighChainAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class WhiteKnobLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

        juce::Label* createSliderTextBox (juce::Slider&) override;
    };

    HighChainAudioProcessor& audioProcessor;
    WhiteKnobLookAndFeel knobLook;

    juce::Slider inputSlider, sparkSlider, outputSlider;
    juce::Label inputLabel, sparkLabel, outputLabel;
    juce::Label titleLabel, subTitleLabel, brandLabel;
    juce::Label inputScaleLabel, sparkScaleLabel, outputScaleLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment, sparkAttachment, outputAttachment;

    void setupSlider (juce::Slider&, juce::Label&, const juce::String& labelText);
    void setupLabel (juce::Label&, const juce::String&, float size,
                     juce::Colour colour, bool bold = false,
                     juce::Justification justification = juce::Justification::centred);

    void drawLogo (juce::Graphics&, juce::Rectangle<float> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighChainAudioProcessorEditor)
};

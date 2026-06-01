#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HighChainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit HighChainAudioProcessorEditor (HighChainAudioProcessor&);
    ~HighChainAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HighChainAudioProcessor& audioProcessor;

    juce::Slider inputSlider, sparkSlider, outputSlider;
    juce::ToggleButton deltaButton;
    juce::Label inputLabel, sparkLabel, outputLabel, titleLabel, brandLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment, sparkAttachment, outputAttachment;
    std::unique_ptr<ButtonAttachment> deltaAttachment;

    void setupSlider (juce::Slider&, juce::Label&, const juce::String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighChainAudioProcessorEditor)
};

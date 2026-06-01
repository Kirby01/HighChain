#include "PluginEditor.h"

HighChainAudioProcessorEditor::HighChainAudioProcessorEditor (HighChainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (460, 260);

    titleLabel.setText ("HighChain", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont (juce::Font (28.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    brandLabel.setText ("Harmonious Records", juce::dontSendNotification);
    brandLabel.setJustificationType (juce::Justification::centred);
    brandLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
    brandLabel.setFont (juce::Font (13.0f));
    addAndMakeVisible (brandLabel);

    setupSlider (inputSlider,  inputLabel,  "Input");
    setupSlider (sparkSlider,  sparkLabel,  "Spark");
    setupSlider (outputSlider, outputLabel, "Output");

    deltaButton.setButtonText ("Delta");
    deltaButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (deltaButton);

    inputAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "input",  inputSlider);
    sparkAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "spark",  sparkSlider);
    outputAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
    deltaAttachment  = std::make_unique<ButtonAttachment> (audioProcessor.apvts, "delta", deltaButton);
}

void HighChainAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 74, 20);
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    label.setFont (juce::Font (15.0f, juce::Font::bold));
    addAndMakeVisible (label);
}

void HighChainAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (18, 18, 20));

    auto r = getLocalBounds().toFloat().reduced (12.0f);
    g.setColour (juce::Colour (38, 38, 42));
    g.fillRoundedRectangle (r, 16.0f);

    g.setColour (juce::Colours::orange.withAlpha (0.7f));
    g.drawRoundedRectangle (r, 16.0f, 2.0f);
}

void HighChainAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (18);

    titleLabel.setBounds (bounds.removeFromTop (38));
    brandLabel.setBounds (bounds.removeFromTop (22));
    bounds.removeFromTop (10);

    auto knobArea = bounds.removeFromTop (125);
    auto w = knobArea.getWidth() / 3;

    auto inputArea  = knobArea.removeFromLeft (w);
    auto sparkArea  = knobArea.removeFromLeft (w);
    auto outputArea = knobArea;

    inputLabel.setBounds  (inputArea.removeFromTop (22));
    sparkLabel.setBounds  (sparkArea.removeFromTop (22));
    outputLabel.setBounds (outputArea.removeFromTop (22));

    inputSlider.setBounds  (inputArea.reduced (8));
    sparkSlider.setBounds  (sparkArea.reduced (8));
    outputSlider.setBounds (outputArea.reduced (8));

    deltaButton.setBounds (getWidth() / 2 - 45, getHeight() - 42, 90, 24);
}

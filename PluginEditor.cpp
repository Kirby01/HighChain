#include "PluginEditor.h"

namespace
{
    const juce::Colour textDark     (36, 42, 45);
    const juce::Colour textMid      (125, 128, 132);
    const juce::Colour lineLight    (225, 227, 230);
    const juce::Colour purple       (92, 84, 255);
    const juce::Colour purpleLight  (126, 113, 255);

    juce::String spaced (const juce::String& s)
    {
        juce::String out;
        for (int i = 0; i < s.length(); ++i)
        {
            out << s.substring (i, i + 1);
            if (i != s.length() - 1)
                out << " ";
        }
        return out;
    }
}

HighChainAudioProcessorEditor::HighChainAudioProcessorEditor (HighChainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (820, 500);

    setupLabel (titleLabel, "H I G H C H A I N", 34.0f, textDark, false);
    setupLabel (subTitleLabel, "I N T E L L I G E N T   H I G H   F R E Q U E N C Y   E N H A N C E M E N T", 11.5f, textMid, false);
    setupLabel (brandLabel, spaced ("HARMONIOUS RECORDS"), 11.0f, juce::Colour (170, 173, 176), false);

    inputSlider.setName ("inputSlider");
    sparkSlider.setName ("sparkSlider");
    outputSlider.setName ("outputSlider");

    setupSlider (inputSlider,  inputLabel,  "I N P U T");
    setupSlider (sparkSlider,  sparkLabel,  "S P A R K");
    setupSlider (outputSlider, outputLabel, "O U T P U T");

    inputSlider.setTextValueSuffix (" dB");
    outputSlider.setTextValueSuffix (" dB");

    // Spark is 0.0 - 10.0 internally. Show it as 0 - 100%.
    sparkSlider.textFromValueFunction = [] (double v) { return juce::String (v * 10.0, 0) + " %"; };
    sparkSlider.valueFromTextFunction = [] (const juce::String& s) { return s.retainCharacters ("0123456789.-").getDoubleValue() / 10.0; };

    setupLabel (inputScaleLabel,  "-24          0          +24", 12.0f, textMid, false);
    setupLabel (sparkScaleLabel,  "S O F T                 B R I G H T", 11.0f, textMid, false);
    setupLabel (outputScaleLabel, "-24          0          +24", 12.0f, textMid, false);

    inputAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "input",  inputSlider);
    sparkAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "spark",  sparkSlider);
    outputAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
}

HighChainAudioProcessorEditor::~HighChainAudioProcessorEditor()
{
    inputSlider.setLookAndFeel (nullptr);
    sparkSlider.setLookAndFeel (nullptr);
    outputSlider.setLookAndFeel (nullptr);
}

void HighChainAudioProcessorEditor::setupLabel (juce::Label& label, const juce::String& text,
                                                float size, juce::Colour colour, bool bold,
                                                juce::Justification justification)
{
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (justification);
    label.setColour (juce::Label::textColourId, colour);
    label.setFont (juce::Font (size, bold ? juce::Font::bold : juce::Font::plain));
    addAndMakeVisible (label);
}

void HighChainAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label,
                                                 const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.18f,
                                juce::MathConstants<float>::pi * 2.82f,
                                true);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 92, 24);
    slider.setLookAndFeel (&knobLook);
    slider.setColour (juce::Slider::textBoxTextColourId, textDark);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    setupLabel (label, labelText, 15.0f, textDark, true);
}

void HighChainAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colours::white, 0.0f, 0.0f,
                             juce::Colour (244, 246, 248), 0.0f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    auto panel = getLocalBounds().toFloat().reduced (18.0f);
    g.setColour (juce::Colours::black.withAlpha (0.07f));
    g.fillRoundedRectangle (panel.translated (0.0f, 5.0f), 20.0f);

    juce::ColourGradient panelGrad (juce::Colours::white, panel.getX(), panel.getY(),
                                    juce::Colour (248, 249, 250), panel.getX(), panel.getBottom(), false);
    g.setGradientFill (panelGrad);
    g.fillRoundedRectangle (panel, 20.0f);

    g.setColour (lineLight);
    g.drawRoundedRectangle (panel, 20.0f, 1.0f);

    drawLogo (g, juce::Rectangle<float> ((float) getWidth() * 0.5f - 29.0f, 46.0f, 58.0f, 52.0f));

    g.setColour (lineLight);
    g.drawLine (118.0f, 156.0f, 278.0f, 156.0f, 1.0f);
    g.drawLine ((float) getWidth() - 278.0f, 156.0f, (float) getWidth() - 118.0f, 156.0f, 1.0f);
}

void HighChainAudioProcessorEditor::resized()
{
    titleLabel.setBounds (0, 96, getWidth(), 46);
    subTitleLabel.setBounds (0, 140, getWidth(), 24);

    const int knobY = 210;
    const int knobSize = 178;
    const int labelY = 180;
    const int valueY = knobY + knobSize + 4;

    auto placeKnob = [&] (juce::Slider& slider, juce::Label& label, juce::Label& scale,
                         int centreX)
    {
        label.setBounds (centreX - 120, labelY, 240, 26);
        slider.setBounds (centreX - knobSize / 2, knobY, knobSize, knobSize + 34);
        scale.setBounds (centreX - 118, valueY + 32, 236, 22);
    };

    placeKnob (inputSlider,  inputLabel,  inputScaleLabel,  205);
    placeKnob (sparkSlider,  sparkLabel,  sparkScaleLabel,  410);
    placeKnob (outputSlider, outputLabel, outputScaleLabel, 615);

    brandLabel.setBounds (0, getHeight() - 54, getWidth(), 24);
}

void HighChainAudioProcessorEditor::drawLogo (juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::Path p;
    const float x = area.getX();
    const float y = area.getY();
    const float w = area.getWidth();
    const float h = area.getHeight();

    p.startNewSubPath (x + w * 0.17f, y + h * 0.50f);
    p.cubicTo (x + w * 0.24f, y + h * 0.18f, x + w * 0.43f, y + h * 0.18f, x + w * 0.50f, y + h * 0.50f);
    p.cubicTo (x + w * 0.57f, y + h * 0.82f, x + w * 0.76f, y + h * 0.82f, x + w * 0.83f, y + h * 0.50f);

    p.startNewSubPath (x + w * 0.17f, y + h * 0.50f);
    p.cubicTo (x + w * 0.24f, y + h * 0.82f, x + w * 0.43f, y + h * 0.82f, x + w * 0.50f, y + h * 0.50f);
    p.cubicTo (x + w * 0.57f, y + h * 0.18f, x + w * 0.76f, y + h * 0.18f, x + w * 0.83f, y + h * 0.50f);

    juce::ColourGradient grad (purple, area.getX(), area.getY(), purpleLight, area.getRight(), area.getBottom(), false);
    g.setGradientFill (grad);
    g.strokePath (p, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void HighChainAudioProcessorEditor::WhiteKnobLookAndFeel::drawRotarySlider
    (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
     float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (10.0f, 14.0f);
    const float size = juce::jmin (bounds.getWidth(), bounds.getHeight() - 28.0f);
    auto knob = juce::Rectangle<float> (bounds.getCentreX() - size * 0.5f,
                                       bounds.getY() + 10.0f,
                                       size, size).reduced (8.0f);
    const auto centre = knob.getCentre();
    const float radius = knob.getWidth() * 0.5f;

    const int dots = 23;
    for (int i = 0; i < dots; ++i)
    {
        const float t = (float) i / (float) (dots - 1);
        const float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        const bool active = t <= sliderPos;
        const float dotR = (i == dots / 2 ? 3.2f : 2.0f);
        const float rr = radius + 18.0f;
        const float dx = centre.x + std::cos (a - juce::MathConstants<float>::halfPi) * rr;
        const float dy = centre.y + std::sin (a - juce::MathConstants<float>::halfPi) * rr;

        auto c = slider.getName() == "sparkSlider" && active ? purple : juce::Colour (145, 148, 152);
        g.setColour (c.withAlpha (active ? 0.95f : 0.48f));
        g.fillEllipse (dx - dotR, dy - dotR, dotR * 2.0f, dotR * 2.0f);
    }

    g.setColour (juce::Colours::black.withAlpha (0.11f));
    g.fillEllipse (knob.translated (3.0f, 7.0f));

    juce::ColourGradient face (juce::Colours::white, knob.getX(), knob.getY(),
                               juce::Colour (236, 238, 240), knob.getX(), knob.getBottom(), false);
    g.setGradientFill (face);
    g.fillEllipse (knob);

    g.setColour (juce::Colour (220, 223, 226));
    g.drawEllipse (knob, 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.drawEllipse (knob.reduced (6.0f), 1.0f);

    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path pointer;
    pointer.addRoundedRectangle (-2.0f, -radius * 0.70f, 4.0f, radius * 0.32f, 2.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (textDark);
    g.fillPath (pointer);
}

juce::Label* HighChainAudioProcessorEditor::WhiteKnobLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = juce::LookAndFeel_V4::createSliderTextBox (slider);
    label->setJustificationType (juce::Justification::centred);
    label->setFont (juce::Font (15.0f));
    label->setColour (juce::Label::textColourId, textDark);
    label->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    return label;
}

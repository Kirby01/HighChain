#include "PluginProcessor.h"
#include "PluginEditor.h"

HighChainAudioProcessor::HighChainAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout HighChainAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>("input",  "Input",  juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("spark",  "Spark",  juce::NormalisableRange<float>(0.0f, 10.0f, 0.001f), 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("output", "Output", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));

    return { params.begin(), params.end() };
}

void HighChainAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    coreL.reset();
    coreR.reset();
}

bool HighChainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == out;
}

double HighChainAudioProcessor::processChannelSample (double input, ChannelCore& ch, double amount, double dcCoef)
{
    // ReaJS 2x upsample per channel:
    // xA = 0.5 * (lastIn + in); xB = in; lastIn = in;
    const double xA = 0.5 * (ch.lastIn + input);
    const double xB = input;
    ch.lastIn = input;

    const double motionA = ch.processSubSample (xA);
    const double yA = xA + motionA * amount;

    const double motionB = ch.processSubSample (xB);
    const double yB = xB + motionB * amount;

    const double y = 0.5 * (yA + yB);

    const double dcOut = y - ch.dcX1 + dcCoef * ch.dcY1;
    ch.dcX1 = y;
    ch.dcY1 = dcOut;

    return dcOut;
}

void HighChainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const double inGain  = juce::Decibels::decibelsToGain ((double) apvts.getRawParameterValue("input")->load());
    const double outGain = juce::Decibels::decibelsToGain ((double) apvts.getRawParameterValue("output")->load());
    const double spark   = (double) apvts.getRawParameterValue("spark")->load();

    // Exact port of the working ReaJS amount line:
    // amount = slider2 * 0.25;
    const double amount = spark * 0.25;

    // Same 10 Hz-ish DC blocker as the ReaJS/JUCE versions.
    const double dcCoef = std::exp (-2.0 * juce::MathConstants<double>::pi * 10.0 / currentSampleRate);

    double blockMeter = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const double inL = buffer.getSample (0, i) * inGain;
        const double inR = (numChannels > 1 ? buffer.getSample (1, i) : inL) * inGain;

        // True dual-mono processing: each side gets its own HighChain state.
        // This avoids the previous mono shortcut: x0 = 0.5 * (L + R), then adding
        // the same generated layer back to both channels, which could narrow or shift
        // the stereo image.
        const double dcOutL = processChannelSample (inL, coreL, amount, dcCoef);
        const double dcOutR = (numChannels > 1)
                                ? processChannelSample (inR, coreR, amount, dcCoef)
                                : dcOutL;

        blockMeter = juce::jmax (blockMeter, std::abs (dcOutL - inL));
        if (numChannels > 1)
            blockMeter = juce::jmax (blockMeter, std::abs (dcOutR - inR));

        buffer.setSample (0, i, (float) (dcOutL * outGain));

        if (numChannels > 1)
            buffer.setSample (1, i, (float) (dcOutR * outGain));
    }

    const float meterTarget = juce::jlimit (0.0f, 1.0f, (float) (blockMeter * 5.0));
    const float oldMeter = sparkMeter.load();
    sparkMeter.store (juce::jmax (meterTarget, oldMeter * 0.92f));
}

void HighChainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void HighChainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* HighChainAudioProcessor::createEditor()
{
    return new HighChainAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HighChainAudioProcessor();
}

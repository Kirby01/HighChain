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
    params.push_back (std::make_unique<juce::AudioParameterFloat>("spark",  "Spark",  juce::NormalisableRange<float>(0.0f, 10.0f, 0.001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("output", "Output", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>("delta", "Delta", false));

    return { params.begin(), params.end() };
}

void HighChainAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    core.reset();
}

bool HighChainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void HighChainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const double inGain  = juce::Decibels::decibelsToGain ((double) apvts.getRawParameterValue("input")->load());
    const double outGain = juce::Decibels::decibelsToGain ((double) apvts.getRawParameterValue("output")->load());
    const double spark   = (double) apvts.getRawParameterValue("spark")->load();
    const bool deltaMode = apvts.getRawParameterValue("delta")->load() > 0.5f;

    const double dcCoef = std::exp (-2.0 * juce::MathConstants<double>::pi * 10.0 / currentSampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        const double inL = buffer.getSample (0, i) * inGain;
        const double inR = (numChannels > 1 ? buffer.getSample (1, i) : inL) * inGain;

        const double x0 = 0.5 * (inL + inR);
        const double x1 = 0.5 * (core.lastIn + x0);
        core.lastIn = x0;

        const double motion1 = core.processSubSample (x1);
        const double motion2 = core.processSubSample (x0);

        const double layer = 0.5 * (motion1 + motion2) * spark;

        double wetL = deltaMode ? layer : inL + layer;
        double wetR = deltaMode ? layer : inR + layer;

        const double dcOutL = wetL - core.dcX1L + dcCoef * core.dcY1L;
        const double dcOutR = wetR - core.dcX1R + dcCoef * core.dcY1R;

        core.dcX1L = wetL; core.dcY1L = dcOutL;
        core.dcX1R = wetR; core.dcY1R = dcOutR;

        buffer.setSample (0, i, (float) (dcOutL * outGain));
        if (numChannels > 1)
            buffer.setSample (1, i, (float) (dcOutR * outGain));
    }
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

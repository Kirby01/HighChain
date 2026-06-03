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

        // ReaJS:
        // in = spl0 * inGain;
        //
        // JUCE keeps stereo audio, but uses the same mono HighChain control/audio source.
        const double x0 = 0.5 * (inL + inR);

        // ReaJS 2x upsample:
        // xA = 0.5 * (lastIn + in);
        // xB = in;
        // lastIn = in;
        const double xA = 0.5 * (core.lastIn + x0);
        const double xB = x0;
        core.lastIn = x0;

        // ReaJS substep A:
        // flip = -flip;
        // xf = xA * flip;
        // motionA = xf - prev;
        // prev = xf;
        // layerA = motionA * amount;
        // yA = xA + layerA;
        const double motionA = core.processSubSample (xA);
        const double layerA = motionA * amount;
        const double yA = xA + layerA;

        // ReaJS substep B:
        const double motionB = core.processSubSample (xB);
        const double layerB = motionB * amount;
        const double yB = xB + layerB;

        // Working version: direct average downsample, no extra downLP smoothing.
        const double y = 0.5 * (yA + yB);

        // Convert the mono processed result into a layer and add it to both stereo channels.
        const double layer = y - x0;
        blockMeter = juce::jmax (blockMeter, std::abs (layer));

        const double wetL = inL + layer;
        const double wetR = inR + layer;

        const double dcOutL = wetL - core.dcX1L + dcCoef * core.dcY1L;
        const double dcOutR = wetR - core.dcX1R + dcCoef * core.dcY1R;

        core.dcX1L = wetL; core.dcY1L = dcOutL;
        core.dcX1R = wetR; core.dcY1R = dcOutR;

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

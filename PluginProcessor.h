#pragma once
#include <JuceHeader.h>
#include <atomic>

class HighChainAudioProcessor : public juce::AudioProcessor
{
public:
    HighChainAudioProcessor();
    ~HighChainAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    float getSparkMeter() const { return sparkMeter.load(); }

private:
    struct Core
    {
        double prev = 0.0;
        double flip = 1.0;
        double lastIn = 0.0;

        double dcX1L = 0.0, dcY1L = 0.0;
        double dcX1R = 0.0, dcY1R = 0.0;

        void reset()
        {
            prev = 0.0;
            flip = 1.0;
            lastIn = 0.0;

            dcX1L = dcY1L = 0.0;
            dcX1R = dcY1R = 0.0;
        }

        double processSubSample (double x)
        {
            flip = -flip;

            const double xf = x * flip;
            const double motion = xf - prev;
            prev = xf;

            return motion;
        }
    };

    Core core;
    double currentSampleRate = 44100.0;
    std::atomic<float> sparkMeter { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighChainAudioProcessor)
};

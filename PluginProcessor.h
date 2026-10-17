#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "signalsmith-stretch.h"
#include <vector>
#include <cmath>
#include <atomic>

class CyberGodVocalAudioProcessor : public juce::AudioProcessor
{
public:
    CyberGodVocalAudioProcessor();
    ~CyberGodVocalAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "CyberGodVocal"; }
    bool acceptsMidi() const override { return true; } // Accepts MIDI note override in Ableton
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

    juce::AudioProcessorValueTreeState apvts;

    // Atomic telemetry for the real-time GUI HUD
    std::atomic<float> uiDetectedFreq { 130.0f };
    std::atomic<float> uiTargetFreq   { 130.0f };
    std::atomic<int>   uiTargetMidi   { 48 };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float detectPitchYIN(double sampleRate);
    float quantizeFreqToScale(float freq, int rootNote, int scaleType, int& outMidiNote);

    double currentSampleRate = 44100.0;
    float currentDetectedFreq = 130.0f;
    float smoothedSemitoneShift = 0.0f;
    float ringModPhase = 0.0f;
    int activeMidiNote = -1;

    // 2048-sample Ring Buffer for rock-solid YIN Pitch Tracking at any DAW buffer size
    static constexpr int pitchBufSize = 2048;
    std::vector<float> pitchRingBuffer;
    std::vector<float> yinLinearBuffer;
    int pitchWritePos = 0;
    int samplesSinceLastDetect = 0;

    // Stereo Haas Widener Delay Buffer for the "God Voice"
    std::vector<float> haasBuffer;
    int haasWritePos = 0;

    // Signalsmith Stretch Phase-Vocoder Engines
    signalsmith::stretch::SignalsmithStretch<float> mainShifter;
    signalsmith::stretch::SignalsmithStretch<float> sub1Shifter;
    signalsmith::stretch::SignalsmithStretch<float> sub2Shifter;

    juce::AudioBuffer<float> inScratchBuffer;
    juce::AudioBuffer<float> mainOutBuffer;
    juce::AudioBuffer<float> sub1OutBuffer;
    juce::AudioBuffer<float> sub2OutBuffer;

    juce::dsp::StateVariableTPTFilter<float> chestFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyberGodVocalAudioProcessor)
};

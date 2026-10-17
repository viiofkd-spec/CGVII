#include "PluginProcessor.h"

//==============================================================================
// Custom Dark Hardware GUI Editor (Embedded so no extra files are needed)
//==============================================================================
class CyberGodEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    CyberGodEditor(CyberGodVocalAudioProcessor& p)
        : AudioProcessorEditor(&p), proc(p)
    {
        setSize(680, 380);

        getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colour(0xff00f0ff));
        getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00f0ff));
        getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff1e2631));
        getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff141920));
        getLookAndFeel().setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00f0ff));

        addAndMakeVisible(rootBox);
        rootBox.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
        rootAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, "Scale_Root", rootBox);

        addAndMakeVisible(scaleBox);
        scaleBox.addItemList({ "Chromatic", "Major", "Minor", "Harmonic Minor" }, 1);
        scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, "Scale_Type", scaleBox);

        setupKnob(retuneSlider,  retuneLabel,  "RETUNE (ms)",   "Retune_Speed",   retuneAttach);
        setupKnob(formantSlider, formantLabel, "FORMANT (st)",  "Formant_Shift",  formantAttach);
        setupKnob(subSlider,     subLabel,     "GOD SUB (%)",   "Sub_Octave_Mix", subAttach);
        setupKnob(robotSlider,   robotLabel,   "CYBER MOD (%)", "Robot_Texture",  robotAttach);
        setupKnob(widthSlider,   widthLabel,   "GOD WIDTH (%)", "Stereo_Width",   widthAttach);
        setupKnob(mixSlider,     mixLabel,     "DRY / WET (%)", "Dry_Wet",        mixAttach);

        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff0b0e14));

        // Top Header Bar & Pitch Monitor HUD
        g.setColour(juce::Colour(0xff131924));
        g.fillRoundedRectangle(20.0f, 15.0f, (float)getWidth() - 40.0f, 95.0f, 8.0f);
        g.setColour(juce::Colour(0xff00f0ff).withAlpha(0.6f));
        g.drawRoundedRectangle(20.0f, 15.0f, (float)getWidth() - 40.0f, 95.0f, 8.0f, 1.5f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
        g.drawText("CYBERGOD // VOCAL ENGINE", 35, 25, 320, 30, juce::Justification::left);

        g.setColour(juce::Colour(0xff8a99ad));
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("0ms YIN PITCH LOCK + SIGNALSMITH FORMANT & SUB-OCTAVE", 35, 52, 360, 20, juce::Justification::left);

        // Live Note HUD
        static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int midi = juce::jlimit(0, 127, proc.uiTargetMidi.load());
        int octave = (midi / 12) - 1;
        juce::String noteStr = juce::String(noteNames[midi % 12]) + juce::String(octave);

        g.setColour(juce::Colour(0xff00f0ff));
        g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
        g.drawText("LOCK: " + noteStr, getWidth() - 230, 24, 190, 40, juce::Justification::right);

        g.setColour(juce::Colour(0xff8a99ad));
        g.setFont(juce::FontOptions(13.0f));
        juce::String freqStr = "IN: " + juce::String(proc.uiDetectedFreq.load(), 1) + " Hz  |  OUT: " + juce::String(proc.uiTargetFreq.load(), 1) + " Hz";
        g.drawText(freqStr, getWidth() - 290, 68, 250, 22, juce::Justification::right);
    }

    void resized() override
    {
        rootBox.setBounds(35, 76, 85, 24);
        scaleBox.setBounds(130, 76, 140, 24);

        int y = 145;
        int w = 104;
        retuneSlider.setBounds(25 + w * 0, y, 95, 165);
        formantSlider.setBounds(25 + w * 1, y, 95, 165);
        subSlider.setBounds(25 + w * 2, y, 95, 165);
        robotSlider.setBounds(25 + w * 3, y, 95, 165);
        widthSlider.setBounds(25 + w * 4, y, 95, 165);
        mixSlider.setBounds(25 + w * 5, y, 95, 165);

        for (auto* lbl : { &retuneLabel, &formantLabel, &subLabel, &robotLabel, &widthLabel, &mixLabel })
            lbl->setBounds(lbl->getAttachedComponent()->getX(), y + 170, 95, 22);
    }

private:
    void timerCallback() override { repaint(20, 15, getWidth() - 40, 95); }

    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& paramID,
                   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attach)
    {
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, paramID, s);

        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        l.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
        l.attachToComponent(&s, false);
    }

    CyberGodVocalAudioProcessor& proc;
    juce::ComboBox rootBox, scaleBox;
    juce::Slider retuneSlider, formantSlider, subSlider, robotSlider, widthSlider, mixSlider;
    juce::Label  retuneLabel,  formantLabel,  subLabel,  robotLabel,  widthLabel,  mixLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rootAttach, scaleAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAttach, formantAttach, subAttach, robotAttach, widthAttach, mixAttach;
};

//==============================================================================
// AudioProcessor Implementation
//==============================================================================
CyberGodVocalAudioProcessor::CyberGodVocalAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CyberGodVocalAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray notes  = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    juce::StringArray scales = { "Chromatic", "Major", "Minor", "Harmonic Minor" };

    params.push_back(std::make_unique<juce::AudioParameterChoice>("Scale_Root", "Scale Root", notes, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("Scale_Type", "Scale Type", scales, 2));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Retune_Speed", "Retune Speed (ms)", juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Formant_Shift", "Formant Shift (st)", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -4.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Sub_Octave_Mix", "Sub-God Octave (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 45.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Robot_Texture", "Robot Synth Texture (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 25.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Stereo_Width", "God Stereo Width (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 60.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("Dry_Wet", "Dry / Wet (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));

    return { params.begin(), params.end() };
}

void CyberGodVocalAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    pitchRingBuffer.assign(pitchBufSize, 0.0f);
    yinLinearBuffer.assign(pitchBufSize, 0.0f);
    pitchWritePos = 0;
    samplesSinceLastDetect = 0;

    // 25ms Haas stereo delay buffer
    haasBuffer.assign((int)(sampleRate * 0.05) + 1024, 0.0f);
    haasWritePos = 0;

    // Signalsmith Stretch configuration
    mainShifter.presetDefault(1, (float)sampleRate);
    sub1Shifter.presetCheaper(1, (float)sampleRate);
    sub2Shifter.presetCheaper(1, (float)sampleRate);

    inScratchBuffer.setSize(1, samplesPerBlock);
    mainOutBuffer.setSize(1, samplesPerBlock);
    sub1OutBuffer.setSize(1, samplesPerBlock);
    sub2OutBuffer.setSize(1, samplesPerBlock);

    setLatencySamples(mainShifter.inputLatency() + mainShifter.outputLatency());

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 1 };
    chestFilter.prepare(spec);
    chestFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    chestFilter.setResonance(1.6f);
}

bool CyberGodVocalAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// Studio-Grade YIN Pitch Estimator (Cumulative Mean Normalized Difference)
float CyberGodVocalAudioProcessor::detectPitchYIN(double sampleRate)
{
    for (int i = 0; i < pitchBufSize; ++i)
        yinLinearBuffer[i] = pitchRingBuffer[(pitchWritePos + i) % pitchBufSize];

    float rms = 0.0f;
    for (int i = 0; i < pitchBufSize; ++i) rms += yinLinearBuffer[i] * yinLinearBuffer[i];
    if (std::sqrt(rms / (float)pitchBufSize) < 0.008f) return currentDetectedFreq;

    int halfSize = pitchBufSize / 2;
    int minLag = juce::jmax(4, (int)(sampleRate / 900.0));
    int maxLag = juce::jmin(halfSize - 1, (int)(sampleRate / 70.0));

    std::vector<float> yinDiff(maxLag + 1, 0.0f);
    float runningSum = 0.0f;
    yinDiff[0] = 1.0f;

    for (int tau = 1; tau <= maxLag; ++tau)
    {
        float deltaSum = 0.0f;
        for (int i = 0; i < halfSize; ++i)
        {
            float d = yinLinearBuffer[i] - yinLinearBuffer[i + tau];
            deltaSum += d * d;
        }
        runningSum += deltaSum;
        yinDiff[tau] = (runningSum > 0.0f) ? (deltaSum * (float)tau / runningSum) : 1.0f;
    }

    int bestTau = -1;
    const float threshold = 0.15f;
    for (int tau = minLag; tau < maxLag; ++tau)
    {
        if (yinDiff[tau] < threshold)
        {
            while (tau + 1 < maxLag && yinDiff[tau + 1] < yinDiff[tau])
                ++tau;
            bestTau = tau;
            break;
        }
    }

    if (bestTau == -1)
    {
        float minVal = 100.0f;
        for (int tau = minLag; tau < maxLag; ++tau)
        {
            if (yinDiff[tau] < minVal)
            {
                minVal = yinDiff[tau];
                bestTau = tau;
            }
        }
    }

    return (bestTau > 0) ? (float)(sampleRate / (double)bestTau) : currentDetectedFreq;
}

float CyberGodVocalAudioProcessor::quantizeFreqToScale(float freq, int rootNote, int scaleType, int& outMidiNote)
{
    float midiNote = 69.0f + 12.0f * std::log2(freq / 440.0f);
    int roundedMidi = (int)std::round(midiNote);

    const bool scales[4][12] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Chromatic
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, // Major
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0}, // Natural Minor
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1}  // Harmonic Minor
    };

    int bestNote = roundedMidi;
    int minDist = 12;

    for (int offset = -6; offset <= 6; ++offset)
    {
        int candidate = roundedMidi + offset;
        int degree = ((candidate - rootNote) % 12 + 12) % 12;
        if (scales[scaleType][degree] && std::abs(offset) < minDist)
        {
            minDist = std::abs(offset);
            bestNote = candidate;
        }
    }
    outMidiNote = bestNote;
    return 440.0f * std::pow(2.0f, (bestNote - 69.0f) / 12.0f);
}

void CyberGodVocalAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    // Check incoming MIDI Notes for Live Pitch Override
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) activeMidiNote = msg.getNoteNumber();
        else if (msg.isNoteOff() && msg.getNoteNumber() == activeMidiNote) activeMidiNote = -1;
    }

    if (inScratchBuffer.getNumSamples() < numSamples)
    {
        inScratchBuffer.setSize(1, numSamples, false, false, true);
        mainOutBuffer.setSize(1, numSamples, false, false, true);
        sub1OutBuffer.setSize(1, numSamples, false, false, true);
        sub2OutBuffer.setSize(1, numSamples, false, false, true);
    }

    int scaleRoot      = (int)*apvts.getRawParameterValue("Scale_Root");
    int scaleType      = (int)*apvts.getRawParameterValue("Scale_Type");
    float retuneMs     = *apvts.getRawParameterValue("Retune_Speed");
    float formantSt    = *apvts.getRawParameterValue("Formant_Shift");
    float subMix       = *apvts.getRawParameterValue("Sub_Octave_Mix") * 0.01f;
    float robotTexture = *apvts.getRawParameterValue("Robot_Texture") * 0.01f;
    float stereoWidth  = *apvts.getRawParameterValue("Stereo_Width") * 0.01f;
    float wetMix       = *apvts.getRawParameterValue("Dry_Wet") * 0.01f;

    inScratchBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    const float* inPtr = inScratchBuffer.getReadPointer(0);

    // Feed Rolling Pitch Ring Buffer
    for (int i = 0; i < numSamples; ++i)
    {
        pitchRingBuffer[pitchWritePos] = inPtr[i];
        pitchWritePos = (pitchWritePos + 1) % pitchBufSize;
    }
    samplesSinceLastDetect += numSamples;
    if (samplesSinceLastDetect >= 256)
    {
        currentDetectedFreq = detectPitchYIN(currentSampleRate);
        samplesSinceLastDetect = 0;
    }

    int targetMidi = 60;
    float targetFreq = quantizeFreqToScale(currentDetectedFreq, scaleRoot, scaleType, targetMidi);
    if (activeMidiNote >= 0)
    {
        targetMidi = activeMidiNote;
        targetFreq = 440.0f * std::pow(2.0f, (activeMidiNote - 69.0f) / 12.0f);
    }

    uiDetectedFreq.store(currentDetectedFreq);
    uiTargetFreq.store(targetFreq);
    uiTargetMidi.store(targetMidi);

    float targetSemitones = juce::jlimit(-24.0f, 24.0f, 12.0f * std::log2(targetFreq / currentDetectedFreq));
    float blockDurationSec = (float)numSamples / (float)currentSampleRate;
    float slewCoeff = (retuneMs <= 0.1f) ? 1.0f : (1.0f - std::exp(-blockDurationSec / (retuneMs * 0.001f)));
    smoothedSemitoneShift += slewCoeff * (targetSemitones - smoothedSemitoneShift);

    // Configure Signalsmith True Pitch + Independent Formant Envelope Shifting
    float tonalityLimit = 8000.0f / (float)currentSampleRate;
    mainShifter.setTransposeSemitones(smoothedSemitoneShift, tonalityLimit);
    mainShifter.setFormantSemitones(formantSt, false);

    sub1Shifter.setTransposeSemitones(smoothedSemitoneShift - 12.0f, tonalityLimit);
    sub1Shifter.setFormantSemitones(formantSt - 2.0f, false);

    sub2Shifter.setTransposeSemitones(smoothedSemitoneShift - 24.0f, tonalityLimit);
    sub2Shifter.setFormantSemitones(formantSt - 4.0f, false);

    const float* inCh[1]   = { inScratchBuffer.getReadPointer(0) };
    float* mainOutCh[1]    = { mainOutBuffer.getWritePointer(0) };
    float* sub1OutCh[1]    = { sub1OutBuffer.getWritePointer(0) };
    float* sub2OutCh[1]    = { sub2OutBuffer.getWritePointer(0) };

    mainShifter.process(inCh, numSamples, mainOutCh, numSamples);
    sub1Shifter.process(inCh, numSamples, sub1OutCh, numSamples);
    sub2Shifter.process(inCh, numSamples, sub2OutCh, numSamples);

    float formantRatio = std::pow(2.0f, formantSt / 12.0f);
    chestFilter.setCutoffFrequency(juce::jlimit(120.0f, 2400.0f, 480.0f * formantRatio));

    int haasDelaySamples = (int)(currentSampleRate * 0.018); // 18ms Haas stereo spread
    int haasSize = (int)haasBuffer.size();

    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        float dry = inPtr[i];
        float mainVox = mainOutCh[0][i];
        float chestRes = chestFilter.processSample(0, mainVox);
        float coreVoice = mainVox + (chestRes * 0.35f);

        float godSubMono = (sub1OutCh[0][i] * 0.8f + sub2OutCh[0][i] * 0.5f) * subMix;

        // Write God Layer into Haas Delay Line for Stereo Width
        haasBuffer[haasWritePos] = godSubMono + (chestRes * 0.5f);
        int haasReadPos = (haasWritePos - haasDelaySamples + haasSize) % haasSize;
        float delayedGodR = haasBuffer[haasReadPos];
        haasWritePos = (haasWritePos + 1) % haasSize;

        // Ring Modulator + Sub-Harmonic Synth Carrier locked to Target Note
        ringModPhase += targetFreq / (float)currentSampleRate;
        if (ringModPhase >= 1.0f) ringModPhase -= 1.0f;
        float carrier = std::sin(2.0f * juce::MathConstants<float>::pi * ringModPhase);
        float synthLayer = (coreVoice * carrier) * robotTexture;

        float wetL = std::tanh((coreVoice + godSubMono + synthLayer) * 1.25f);
        float wetR = std::tanh((coreVoice + (godSubMono * (1.0f - stereoWidth * 0.5f)) + (delayedGodR * stereoWidth) + synthLayer) * 1.25f);

        outL[i] = (dry * (1.0f - wetMix)) + (wetL * wetMix);
        if (outR != nullptr)
            outR[i] = (dry * (1.0f - wetMix)) + (wetR * wetMix);
    }
}

juce::AudioProcessorEditor* CyberGodVocalAudioProcessor::createEditor()
{
    return new CyberGodEditor (*this);
}

void CyberGodVocalAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CyberGodVocalAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CyberGodVocalAudioProcessor();
}

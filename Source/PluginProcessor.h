#pragma once
#include <JuceHeader.h>
#include "RealLabDSP.h"

class RealLabProcessor : public juce::AudioProcessor
{
public:
    struct PresetEntry
    {
        juce::String name;
        juce::String genre;        // Afrobeat, Pop, Trap, Drill, Zouk or Meus
        juce::String instrument;   // Piano, Guitarra, Violão, Flauta, Baixo
        juce::String image;        // artwork file name, empty if none
        juce::File file;           // empty for factory presets
        int factoryIndex = -1;     // >= 0 for factory presets
    };

    RealLabProcessor();
    ~RealLabProcessor() override;

    void prepareToPlay (double newSampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Real Lab"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::vector<PresetEntry> getPresetList() const;
    void applyPreset (const PresetEntry& entry);
    void savePresetTo (const juce::File& file);
    static juce::File getUserPresetFolder();

    bool loadSampleFolder (const juce::File& fileOrFolder);
    void clearSamples();
    juce::String getSampleStatus() const { return sampleStatus; }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void setParam (const juce::String& id, float value);
    void applyFactoryPreset (int index);
    void loadPresetFile (const juce::File& file);
    void updateVoices();
    void applyEffects (juce::AudioBuffer<float>& buffer);

    juce::Synthesiser synth, sampler;

    RealLab::StateVariableFilter filterL, filterR;
    RealLab::StereoDelayLine vibrato, delay;
    juce::Reverb reverb;

    double sr = 44100.0;
    float vibPhase = 0.0f, gatePhase = 0.0f, gateGain = 1.0f;
    juce::String sampleStatus { "Synth mode: no samples loaded" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RealLabProcessor)
};

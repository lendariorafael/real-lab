#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace RealLab
{
//==============================================================================
// Built-in synth, used when no sample library is loaded.
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    void setSettings (int waveform, float detuneCents, float attackSec, float releaseSec)
    {
        wave        = waveform;
        detuneRatio = std::pow (2.0f, detuneCents / 1200.0f);
        adsrParams.attack  = juce::jmax (0.001f, attackSec);
        adsrParams.decay   = 0.15f;
        adsrParams.sustain = 0.85f;
        adsrParams.release = juce::jmax (0.005f, releaseSec);
        adsr.setParameters (adsrParams);
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        freq   = (float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        level  = velocity * 0.22f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        adsr.setParameters (adsrParams);
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        adsr.noteOff();
        if (! allowTailOff || ! adsr.isActive())
            clearCurrentNote();
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void setCurrentPlaybackSampleRate (double newRate) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
        playbackRate = (float) newRate;
        adsr.setSampleRate (newRate);
    }

    void renderNextBlock (juce::AudioBuffer<float>& output, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        const float inc1 = freq / playbackRate;
        const float inc2 = inc1 * detuneRatio;

        for (int i = 0; i < numSamples; ++i)
        {
            const float env    = adsr.getNextSample();
            const float sample = 0.5f * (osc (phase1) + osc (phase2)) * env * level;

            phase1 += inc1; if (phase1 >= 1.0f) phase1 -= 1.0f;
            phase2 += inc2; if (phase2 >= 1.0f) phase2 -= 1.0f;

            for (int ch = 0; ch < output.getNumChannels(); ++ch)
                output.addSample (ch, startSample + i, sample);
        }

        if (! adsr.isActive())
            clearCurrentNote();
    }

private:
    float osc (float p) const noexcept
    {
        switch (wave)
        {
            case 0:  return 2.0f * p - 1.0f;                       // saw
            case 1:  return p < 0.5f ? 1.0f : -1.0f;               // square
            case 2:  return 4.0f * std::abs (p - 0.5f) - 1.0f;     // triangle
            default: return std::sin (juce::MathConstants<float>::twoPi * p);
        }
    }

    int wave = 0;
    float detuneRatio = 1.0f, freq = 440.0f, level = 0.0f;
    float phase1 = 0.0f, phase2 = 0.0f, playbackRate = 44100.0f;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
};

//==============================================================================
// Zavalishin TPT state-variable low-pass filter.
struct StateVariableFilter
{
    void setup (float cutoffHz, float q, double sampleRate) noexcept
    {
        const float fc = juce::jlimit (20.0f, 0.45f * (float) sampleRate, cutoffHz);
        const float g  = std::tan (juce::MathConstants<float>::pi * fc / (float) sampleRate);
        const float k  = 1.0f / juce::jmax (0.1f, q);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    void reset() noexcept { ic1 = 0.0f; ic2 = 0.0f; }

    float process (float x) noexcept
    {
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        return v2;
    }

private:
    float ic1 = 0.0f, ic2 = 0.0f, a1 = 1.0f, a2 = 0.0f, a3 = 0.0f;
};

//==============================================================================
// Stereo circular delay buffer with linear interpolation (used by vibrato and delay).
struct StereoDelayLine
{
    void prepare (int maxSamples)
    {
        size = juce::jmax (8, maxSamples);
        for (auto& b : buffers)
            b.assign ((size_t) size, 0.0f);
        pos = 0;
    }

    float read (int ch, float delaySamples) const noexcept
    {
        const float d = juce::jlimit (1.0f, (float) size - 2.0f, delaySamples);
        float r = (float) pos - d;
        if (r < 0.0f)
            r += (float) size;

        const float frac = r - std::floor (r);
        int i0 = (int) r;
        if (i0 >= size) i0 -= size;
        const int i1 = (i0 + 1) % size;

        const auto& b = buffers[(size_t) ch];
        return b[(size_t) i0] + (b[(size_t) i1] - b[(size_t) i0]) * frac;
    }

    void write (int ch, float x) noexcept
    {
        buffers[(size_t) ch][(size_t) pos] = x;
    }

    void advance() noexcept
    {
        pos = (pos + 1) % size;
    }

private:
    std::array<std::vector<float>, 2> buffers;
    int size = 8;
    int pos = 0;
};
} // namespace RealLab

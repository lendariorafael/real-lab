#pragma once
#include "PluginProcessor.h"
#include <map>

namespace RLColours
{
    const juce::Colour bgTop    (0xff0d2c34);
    const juce::Colour bgBottom (0xff061017);
    const juce::Colour panel    (0xff10262e);
    const juce::Colour panel2   (0xff1b3c46);
    const juce::Colour outline  (0xff2b5d69);
    const juce::Colour accent   (0xffff7a45);
    const juce::Colour accent2  (0xff2ec4b6);
    const juce::Colour text     (0xffeaf6f4);
    const juce::Colour dim      (0xff8fb3b0);
}

//==============================================================================
class RealLabLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RealLabLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId, RLColours::panel2);
        setColour (juce::ComboBox::textColourId, RLColours::text);
        setColour (juce::ComboBox::outlineColourId, RLColours::outline);
        setColour (juce::ComboBox::arrowColourId, RLColours::accent);
        setColour (juce::PopupMenu::backgroundColourId, RLColours::panel);
        setColour (juce::PopupMenu::textColourId, RLColours::text);
        setColour (juce::Label::textColourId, RLColours::dim);
        setColour (juce::TextButton::buttonColourId, RLColours::panel2);
        setColour (juce::TextButton::textColourOffId, RLColours::text);
        setColour (juce::TextButton::textColourOnId, RLColours::text);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                           float startAngle, float endAngle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (3.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const float angle = startAngle + pos * (endAngle - startAngle);

        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillEllipse (centre.x - radius + 2.0f, centre.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

        juce::ColourGradient body (RLColours::panel2, centre.x - radius * 0.6f, centre.y - radius * 0.8f,
                                   RLColours::panel, centre.x + radius, centre.y + radius, true);
        g.setGradientFill (body);
        g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (RLColours::outline);
        g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        const float arcRadius = radius - 4.0f;
        const auto stroke = juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
        g.setColour (juce::Colour (0xff071419));
        g.strokePath (track, stroke);

        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour (RLColours::accent);
        g.strokePath (value, stroke);

        g.setColour (RLColours::text);
        g.drawLine (juce::Line<float> (centre.x + std::sin (angle) * radius * 0.25f,
                                       centre.y - std::cos (angle) * radius * 0.25f,
                                       centre.x + std::sin (angle) * radius * 0.75f,
                                       centre.y - std::cos (angle) * radius * 0.75f), 2.5f);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
                           juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearVertical)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            return;
        }

        const float cx = (float) x + (float) width * 0.5f;

        g.setColour (RLColours::panel2.darker (0.3f));
        g.fillRoundedRectangle (cx - 3.0f, (float) y, 6.0f, (float) height, 3.0f);

        g.setColour (RLColours::accent);
        g.fillRoundedRectangle (cx - 3.0f, sliderPos, 6.0f, (float) (y + height) - sliderPos, 3.0f);

        g.setColour (RLColours::text);
        g.fillRoundedRectangle (cx - 11.0f, sliderPos - 6.0f, 22.0f, 12.0f, 4.0f);

        g.setColour (RLColours::accent2);
        g.fillRect (cx - 7.0f, sliderPos - 0.5f, 14.0f, 1.0f);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool, bool) override
    {
        auto r = button.getLocalBounds().toFloat().reduced (1.0f);
        const bool on = button.getToggleState();

        g.setColour (on ? RLColours::accent : RLColours::panel2);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (on ? RLColours::accent : RLColours::outline);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        g.setColour (on ? juce::Colours::black : RLColours::text);
        g.setFont (juce::Font (12.0f, juce::Font::bold));
        g.drawText (button.getButtonText(), r.toNearestInt(), juce::Justification::centred, false);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool isOver, bool isDown) override
    {
        auto r = button.getLocalBounds().toFloat().reduced (0.5f);
        juce::Colour fill = RLColours::panel2;
        if (button.getToggleState())      fill = RLColours::accent;
        else if (isDown)                  fill = RLColours::accent.darker (0.3f);
        else if (isOver)                  fill = RLColours::panel2.brighter (0.15f);

        g.setColour (fill);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (button.getToggleState() ? RLColours::accent : RLColours::outline);
        g.drawRoundedRectangle (r, 8.0f, 1.0f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        g.setColour (button.getToggleState() ? juce::Colours::black : RLColours::text);
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, false);
    }
};

//==============================================================================
class RealLabEditor : public juce::AudioProcessorEditor,
                      private juce::ListBoxModel
{
public:
    explicit RealLabEditor (RealLabProcessor&);
    ~RealLabEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics&, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;

    struct Control
    {
        juce::String id;
        juce::Slider slider;
        juce::Label label;
        bool fader = false;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    Control* makeControl (const char* paramId, const juce::String& caption, bool isFader);
    Control* findControl (const char* paramId);
    void layoutGrid (juce::Rectangle<int> area, int columns, const std::vector<Control*>& items);
    void refreshLibrary();
    juce::Image getArtwork (const juce::String& fileName);

    RealLabProcessor& processor;
    RealLabLookAndFeel lookAndFeel;

    juce::ListBox library { "Library", this };
    juce::MidiKeyboardComponent keyboard { processor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };

    juce::ComboBox waveBox;
    juce::ToggleButton bypassButton;
    juce::TextButton loadButton { "Load Samples" }, clearButton { "Clear" }, saveButton { "Save Preset" };
    juce::Label sampleLabel;

    std::vector<std::unique_ptr<juce::TextButton>> genreButtons;
    int currentGenre = 0;

    std::vector<std::unique_ptr<Control>> controls;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::FileChooser> chooser;

    std::vector<RealLabProcessor::PresetEntry> allPresets;
    std::vector<int> visible;
    std::map<juce::String, juce::Image> artworkCache;
    int selectedRow = -1;

    juce::Rectangle<int> groups[9];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RealLabEditor)
};

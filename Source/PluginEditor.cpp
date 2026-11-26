#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
    const char* const kGenres[] = { "Afrobeat", "Pop", "Trap", "Drill", "Zouk", "Meus" };
    constexpr int kNumGenres = 6;

    // Small vector icons, drawn in code, one per effect or section.
    void drawIcon (juce::Graphics& g, const juce::String& kind, juce::Rectangle<float> r)
    {
        g.setColour (RLColours::accent2);
        auto X = [&] (float t) { return r.getX() + t * r.getWidth(); };
        auto Y = [&] (float t) { return r.getY() + t * r.getHeight(); };
        const auto stroke = juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        juce::Path p;

        if (kind == "filter")
        {
            p.startNewSubPath (X (0.0f), Y (0.25f));
            p.lineTo (X (0.4f), Y (0.25f));
            p.quadraticTo (X (0.6f), Y (0.25f), X (0.85f), Y (0.92f));
            p.lineTo (X (1.0f), Y (0.95f));
            g.strokePath (p, stroke);
        }
        else if (kind == "vibrato")
        {
            p.startNewSubPath (X (0.0f), Y (0.5f));
            for (int i = 1; i <= 32; ++i)
            {
                const float t = (float) i / 32.0f;
                p.lineTo (X (t), Y (0.5f - 0.38f * std::sin (t * juce::MathConstants<float>::twoPi * 2.0f)));
            }
            g.strokePath (p, stroke);
        }
        else if (kind == "gate")
        {
            p.startNewSubPath (X (0.0f), Y (0.2f));
            p.lineTo (X (0.2f), Y (0.2f)); p.lineTo (X (0.2f), Y (0.8f));
            p.lineTo (X (0.4f), Y (0.8f)); p.lineTo (X (0.4f), Y (0.2f));
            p.lineTo (X (0.6f), Y (0.2f)); p.lineTo (X (0.6f), Y (0.8f));
            p.lineTo (X (0.8f), Y (0.8f)); p.lineTo (X (0.8f), Y (0.2f));
            p.lineTo (X (1.0f), Y (0.2f));
            g.strokePath (p, stroke);
        }
        else if (kind == "character")
        {
            p.startNewSubPath (X (0.0f), Y (0.5f));
            for (int i = 1; i <= 32; ++i)
            {
                const float t = (float) i / 32.0f;
                const float v = std::tanh (3.0f * (2.0f * t - 1.0f)) / std::tanh (3.0f);
                p.lineTo (X (t), Y (0.5f - 0.4f * v));
            }
            g.strokePath (p, stroke);
        }
        else if (kind == "reverb")
        {
            for (int k = 0; k < 3; ++k)
            {
                const float s = 0.35f + 0.28f * (float) k;
                g.setColour (RLColours::accent2.withAlpha (1.0f - 0.28f * (float) k));
                g.drawEllipse (X (0.5f) - s * r.getWidth() * 0.5f, Y (0.5f) - s * r.getHeight() * 0.5f,
                               s * r.getWidth(), s * r.getHeight(), 1.4f);
            }
        }
        else if (kind == "delay")
        {
            g.setColour (RLColours::accent2);
            p.startNewSubPath (X (0.1f), Y (0.5f));
            p.lineTo (X (0.9f), Y (0.5f));
            g.strokePath (p, juce::PathStrokeType (1.0f));
            for (int k = 0; k < 3; ++k)
            {
                const float cx = 0.2f + 0.3f * (float) k;
                const float s = 0.22f - 0.05f * (float) k;
                g.setColour (RLColours::accent2.withAlpha (1.0f - 0.25f * (float) k));
                g.fillEllipse (X (cx) - s * r.getWidth() * 0.5f, Y (0.5f) - s * r.getHeight() * 0.5f,
                               s * r.getWidth(), s * r.getHeight());
            }
        }
        else if (kind == "envelope")
        {
            p.startNewSubPath (X (0.0f), Y (0.95f));
            p.lineTo (X (0.2f), Y (0.05f));
            p.lineTo (X (0.38f), Y (0.35f));
            p.lineTo (X (0.75f), Y (0.35f));
            p.lineTo (X (1.0f), Y (0.95f));
            g.strokePath (p, stroke);
        }
        else if (kind == "wave")
        {
            p.startNewSubPath (X (0.0f), Y (0.85f));
            p.lineTo (X (0.5f), Y (0.15f));
            p.lineTo (X (0.5f), Y (0.85f));
            p.lineTo (X (1.0f), Y (0.15f));
            g.strokePath (p, stroke);
        }
        else if (kind == "master")
        {
            g.drawLine (X (0.5f), Y (0.05f), X (0.5f), Y (0.95f), 1.4f);
            g.fillRoundedRectangle (X (0.3f), Y (0.55f), r.getWidth() * 0.4f, r.getHeight() * 0.18f, 2.0f);
        }
    }

    // Draws an instrument/genre placeholder when there is no artwork yet.
    void drawPlaceholder (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& instrument)
    {
        juce::ColourGradient grad (RLColours::panel2, r.getX(), r.getY(), RLColours::bgBottom, r.getRight(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (RLColours::accent2.withAlpha (0.8f));
        g.setFont (juce::Font (18.0f, juce::Font::bold));
        g.drawText (instrument, r.toNearestInt(), juce::Justification::centred, false);
    }
}

//==============================================================================
RealLabEditor::RealLabEditor (RealLabProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (library);
    library.setRowHeight (124);
    library.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    library.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);

    addAndMakeVisible (keyboard);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, RLColours::accent);
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, RLColours::accent2.withAlpha (0.35f));

    // Genre tabs
    for (int i = 0; i < kNumGenres; ++i)
    {
        auto button = std::make_unique<juce::TextButton> (kGenres[i]);
        button->setClickingTogglesState (true);
        button->setRadioGroupId (7);
        button->onClick = [this, i]
        {
            currentGenre = i;
            selectedRow = -1;
            refreshLibrary();
        };
        addAndMakeVisible (*button);
        genreButtons.push_back (std::move (button));
    }
    genreButtons[0]->setToggleState (true, juce::dontSendNotification);

    juce::StringArray waves { "Saw", "Square", "Triangle", "Sine" };
    waveBox.addItemList (waves, 1);
    addAndMakeVisible (waveBox);
    waveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, "wave", waveBox);

    bypassButton.setButtonText ("Bypass FX");
    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, "bypassFX", bypassButton);

    addAndMakeVisible (loadButton);
    addAndMakeVisible (clearButton);
    addAndMakeVisible (saveButton);

    sampleLabel.setText (processor.getSampleStatus(), juce::dontSendNotification);
    sampleLabel.setFont (juce::Font (12.0f));
    sampleLabel.setColour (juce::Label::textColourId, RLColours::accent2);
    addAndMakeVisible (sampleLabel);

    loadButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Choose a sample or a folder of samples", juce::File {}, "");
        const auto flags = juce::FileBrowserComponent::openMode
                         | juce::FileBrowserComponent::canSelectFiles
                         | juce::FileBrowserComponent::canSelectDirectories;
        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
        {
            const auto result = fc.getResult();
            if (result.exists())
            {
                processor.loadSampleFolder (result);
                sampleLabel.setText (processor.getSampleStatus(), juce::dontSendNotification);
            }
        });
    };

    clearButton.onClick = [this]
    {
        processor.clearSamples();
        sampleLabel.setText (processor.getSampleStatus(), juce::dontSendNotification);
    };

    saveButton.onClick = [this]
    {
        RealLabProcessor::getUserPresetFolder().createDirectory();
        chooser = std::make_unique<juce::FileChooser> ("Save preset",
                    RealLabProcessor::getUserPresetFolder().getChildFile ("Meu Preset.xml"), "*.xml");
        const auto flags = juce::FileBrowserComponent::saveMode
                         | juce::FileBrowserComponent::canSelectFiles
                         | juce::FileBrowserComponent::warnAboutOverwriting;
        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File {})
                return;
            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension (".xml");
            processor.savePresetTo (file);
            refreshLibrary();
        });
    };

    makeControl ("attack",     "Attack",    true);
    makeControl ("release",    "Release",   true);
    makeControl ("volume",     "Volume",    true);
    makeControl ("unison",     "Unison",    false);
    makeControl ("cutoff",     "Filter",    false);
    makeControl ("resonance",  "Resonance", false);
    makeControl ("vibRate",    "Vib Rate",  false);
    makeControl ("vibDepth",   "Vibrato",   false);
    makeControl ("gateRate",   "Gate Rate", false);
    makeControl ("gateAmt",    "Gated",     false);
    makeControl ("character",  "Character", false);
    makeControl ("reverbMix",  "Reverb",    false);
    makeControl ("reverbSize", "Room",      false);
    makeControl ("delayTime",  "Time",      false);
    makeControl ("delayFb",    "Feedback",  false);
    makeControl ("delayMix",   "Delay",     false);

    refreshLibrary();
    setSize (1100, 580);
}

RealLabEditor::~RealLabEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
RealLabEditor::Control* RealLabEditor::makeControl (const char* paramId, const juce::String& caption, bool isFader)
{
    auto c = std::make_unique<Control>();
    c->id = paramId;
    c->fader = isFader;

    c->slider.setSliderStyle (isFader ? juce::Slider::LinearVertical : juce::Slider::RotaryVerticalDrag);
    c->slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    c->slider.setPopupDisplayEnabled (true, false, this);

    c->label.setText (caption, juce::dontSendNotification);
    c->label.setJustificationType (juce::Justification::centred);
    c->label.setFont (juce::Font (11.0f));

    addAndMakeVisible (c->slider);
    addAndMakeVisible (c->label);

    c->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, paramId, c->slider);

    auto* raw = c.get();
    controls.push_back (std::move (c));
    return raw;
}

RealLabEditor::Control* RealLabEditor::findControl (const char* paramId)
{
    for (auto& c : controls)
        if (c->id == paramId)
            return c.get();
    return nullptr;
}

void RealLabEditor::layoutGrid (juce::Rectangle<int> area, int columns, const std::vector<Control*>& items)
{
    const int count = (int) items.size();
    if (count == 0 || columns <= 0)
        return;

    const int rows = (count + columns - 1) / columns;
    const int cellW = area.getWidth() / columns;
    const int cellH = area.getHeight() / rows;

    for (int i = 0; i < count; ++i)
    {
        auto* c = items[(size_t) i];
        if (c == nullptr)
            continue;

        auto cell = juce::Rectangle<int> (area.getX() + (i % columns) * cellW,
                                          area.getY() + (i / columns) * cellH, cellW, cellH);
        c->label.setBounds (cell.removeFromBottom (16));

        if (c->fader)
        {
            c->slider.setBounds (cell.getCentreX() - 15, cell.getY() + 2, 30, cell.getHeight() - 4);
        }
        else
        {
            const int d = juce::jmin (juce::jmin (cell.getWidth(), cell.getHeight()) - 6, 64);
            c->slider.setBounds (cell.getCentreX() - d / 2, cell.getCentreY() - d / 2, d, d);
        }
    }
}

void RealLabEditor::resized()
{
    // Genre tabs under the header
    const int tabW = (1076 - 5 * 8) / kNumGenres;
    for (int i = 0; i < kNumGenres; ++i)
        genreButtons[(size_t) i]->setBounds (12 + i * (tabW + 8), 62, tabW, 28);

    // Panels
    groups[0] = { 12,  100, 200, 348 };   // Library (preset cards)
    groups[1] = { 224, 100, 220, 170 };   // Source
    groups[2] = { 454, 100, 160, 170 };   // Envelope
    groups[3] = { 624, 100, 260, 170 };   // Filter
    groups[4] = { 894, 100, 194, 170 };   // Master
    groups[5] = { 224, 278, 300, 170 };   // Modulation
    groups[6] = { 534, 278, 120, 170 };   // Character
    groups[7] = { 664, 278, 200, 170 };   // Reverb
    groups[8] = { 874, 278, 214, 170 };   // Delay

    loadButton.setBounds (620, 14, 130, 28);
    clearButton.setBounds (758, 14, 80, 28);
    saveButton.setBounds (846, 14, 130, 28);
    sampleLabel.setBounds (260, 16, 350, 24);

    library.setBounds (groups[0].reduced (6, 26));

    waveBox.setBounds (groups[1].getX() + 12, groups[1].getY() + 30, groups[1].getWidth() - 24, 26);
    layoutGrid (groups[1].withTrimmedTop (62).reduced (8, 4), 1, { findControl ("unison") });

    layoutGrid (groups[2].withTrimmedTop (26).reduced (8, 4), 2, { findControl ("attack"), findControl ("release") });

    layoutGrid (groups[3].withTrimmedTop (26).reduced (8, 4), 2, { findControl ("cutoff"), findControl ("resonance") });

    bypassButton.setBounds (groups[4].getX() + 12, groups[4].getY() + 30, groups[4].getWidth() - 24, 26);
    layoutGrid (groups[4].withTrimmedTop (62).reduced (8, 4), 1, { findControl ("volume") });

    layoutGrid (groups[5].withTrimmedTop (26).reduced (8, 4), 4,
                { findControl ("vibRate"), findControl ("vibDepth"), findControl ("gateRate"), findControl ("gateAmt") });

    layoutGrid (groups[6].withTrimmedTop (26).reduced (8, 4), 1, { findControl ("character") });

    layoutGrid (groups[7].withTrimmedTop (26).reduced (8, 4), 2, { findControl ("reverbMix"), findControl ("reverbSize") });

    layoutGrid (groups[8].withTrimmedTop (26).reduced (8, 4), 3,
                { findControl ("delayTime"), findControl ("delayFb"), findControl ("delayMix") });

    keyboard.setBounds (12, 458, 1076, 112);
}

void RealLabEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (RLColours::bgTop, 0.0f, 0.0f, RLColours::bgBottom, 0.0f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillRect (0, 0, getWidth(), 54);
    g.setColour (RLColours::accent);
    g.fillRect (0, 53, getWidth(), 2);

    g.setColour (RLColours::text);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    g.drawText ("REAL LAB", 16, 6, 220, 30, juce::Justification::left);
    g.setColour (RLColours::dim);
    g.setFont (juce::Font (11.0f));
    g.drawText ("SAMPLER / SYNTH / FX", 18, 33, 220, 16, juce::Justification::left);

    static const char* titles[] = { "PRESETS", "SOURCE", "ENVELOPE", "FILTER", "MASTER",
                                    "MODULATION", "CHARACTER", "REVERB", "DELAY" };
    static const char* icons[]  = { "", "wave", "envelope", "filter", "master",
                                    "vibrato", "character", "reverb", "delay" };

    for (int i = 0; i < 9; ++i)
    {
        auto r = groups[i].toFloat();
        g.setColour (RLColours::panel.withAlpha (0.92f));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (RLColours::outline);
        g.drawRoundedRectangle (r, 10.0f, 1.0f);

        int textX = groups[i].getX() + 12;
        if (i > 0)
        {
            drawIcon (g, icons[i], juce::Rectangle<float> (r.getX() + 10.0f, r.getY() + 7.0f, 14.0f, 12.0f));
            textX += 20;
        }

        g.setColour (RLColours::accent2);
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (titles[i], textX, groups[i].getY() + 6, groups[i].getWidth() - 40, 16, juce::Justification::left);
    }
}

//==============================================================================
void RealLabEditor::refreshLibrary()
{
    allPresets = processor.getPresetList();
    visible.clear();

    for (int i = 0; i < (int) allPresets.size(); ++i)
        if (allPresets[(size_t) i].genre == kGenres[currentGenre])
            visible.push_back (i);

    library.updateContent();
    library.repaint();
}

juce::Image RealLabEditor::getArtwork (const juce::String& fileName)
{
    if (fileName.isEmpty())
        return {};

    auto it = artworkCache.find (fileName);
    if (it != artworkCache.end())
        return it->second;

    juce::String resource = fileName;
    resource = resource.replaceCharacter ('.', '_');

    int size = 0;
    const char* data = BinaryData::getNamedResource (resource.toRawUTF8(), size);

    juce::Image image;
    if (data != nullptr && size > 0)
        image = juce::ImageCache::getFromMemory (data, size);

    artworkCache[fileName] = image;
    return image;
}

int RealLabEditor::getNumRows()
{
    return (int) visible.size();
}

void RealLabEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) visible.size()))
        return;

    const auto& preset = allPresets[(size_t) visible[(size_t) row]];
    auto card = juce::Rectangle<float> (4.0f, 4.0f, (float) width - 8.0f, (float) height - 8.0f);

    g.setColour (selected ? RLColours::accent.withAlpha (0.18f) : RLColours::panel2.withAlpha (0.5f));
    g.fillRoundedRectangle (card, 10.0f);
    g.setColour (selected ? RLColours::accent : RLColours::outline);
    g.drawRoundedRectangle (card, 10.0f, selected ? 2.0f : 1.0f);

    auto thumb = card.reduced (6.0f, 6.0f).removeFromTop (card.getHeight() - 36.0f);
    auto artwork = getArtwork (preset.image);
    if (artwork.isValid())
    {
        g.drawImageWithin (artwork, (int) thumb.getX(), (int) thumb.getY(), (int) thumb.getWidth(), (int) thumb.getHeight(),
                           juce::RectanglePlacement::centred, false);
    }
    else
    {
        drawPlaceholder (g, thumb, preset.instrument);
    }

    g.setColour (selected ? RLColours::text : RLColours::dim);
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText (preset.name, (int) card.getX() + 10, (int) card.getBottom() - 30,
                (int) card.getWidth() - 20, 24, juce::Justification::centredLeft, true);
}

void RealLabEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (! juce::isPositiveAndBelow (row, (int) visible.size()))
        return;

    selectedRow = row;
    processor.applyPreset (allPresets[(size_t) visible[(size_t) row]]);
    repaint();
}

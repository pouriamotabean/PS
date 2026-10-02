#include "MainComponent.h"

namespace PSColours
{
    const juce::Colour bg            (0xff0d1117);
    const juce::Colour panel         (0xff161e2b);
    const juce::Colour raised        (0xff1d2633);
    const juce::Colour border        (0xff313942);
    const juce::Colour text          (0xffeef0f2);
    const juce::Colour textDim       (0xff8a94a0);
    const juce::Colour accent        (0xff296095);
    const juce::Colour accentHi      (0xff69a1d0);
    const juce::Colour gold          (0xffd8b52a);
}

MainComponent::MainComponent()
{
    setSize (640, 760);

    titleLabel.setText ("PS", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, PSColours::text);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("Hear exactly what Spotify will play.", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (15.0f)));
    subtitleLabel.setColour (juce::Label::textColourId, PSColours::textDim);
    addAndMakeVisible (subtitleLabel);

    loadButton.onClick = [this] { chooseInputFile(); };
    addAndMakeVisible (loadButton);

    inputFileLabel.setColour (juce::Label::textColourId, PSColours::textDim);
    inputFileLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (inputFileLabel);

    auto setupSectionLabel = [] (juce::Label& l)
    {
        l.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
        l.setColour (juce::Label::textColourId, PSColours::gold);
    };
    setupSectionLabel (targetSectionLabel);
    setupSectionLabel (qualitySectionLabel);
    addAndMakeVisible (targetSectionLabel);
    addAndMakeVisible (qualitySectionLabel);

    for (auto* b : { &targetLoud, &targetNormal, &targetQuiet })
    {
        b->setRadioGroupId (1001);
        b->setColour (juce::ToggleButton::textColourId, PSColours::text);
        b->setColour (juce::ToggleButton::tickColourId, PSColours::accentHi);
        addAndMakeVisible (*b);
    }
    targetNormal.setToggleState (true, juce::dontSendNotification);

    for (auto* b : { &qualityLow, &qualityMedium, &qualityHigh })
    {
        b->setRadioGroupId (1002);
        b->setColour (juce::ToggleButton::textColourId, PSColours::text);
        b->setColour (juce::ToggleButton::tickColourId, PSColours::accentHi);
        addAndMakeVisible (*b);
    }
    qualityHigh.setToggleState (true, juce::dontSendNotification);

    processButton.onClick = [this] { runProcessing(); };
    processButton.setColour (juce::TextButton::buttonColourId, PSColours::accent);
    processButton.setColour (juce::TextButton::textColourOffId, PSColours::text);
    addAndMakeVisible (processButton);

    resultsLabel.setJustificationType (juce::Justification::topLeft);
    resultsLabel.setFont (juce::Font (juce::FontOptions (13.5f)));
    resultsLabel.setColour (juce::Label::textColourId, PSColours::text);
    resultsLabel.setColour (juce::Label::backgroundColourId, PSColours::raised);
    resultsLabel.setText ("Results will appear here after processing.", juce::dontSendNotification);
    addAndMakeVisible (resultsLabel);

    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setColour (juce::Label::textColourId, PSColours::accentHi);
    addAndMakeVisible (statusLabel);

    creditLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    creditLabel.setColour (juce::Label::textColourId, PSColours::textDim);
    creditLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (creditLabel);
}

MainComponent::~MainComponent() = default;

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (PSColours::bg);

    auto panelBounds = getLocalBounds().reduced (18).withTrimmedTop (88).toFloat();
    g.setColour (PSColours::panel);
    g.fillRoundedRectangle (panelBounds, 14.0f);
    g.setColour (PSColours::border);
    g.drawRoundedRectangle (panelBounds, 14.0f, 1.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (70);
    titleLabel.setBounds (header.removeFromTop (42));
    subtitleLabel.setBounds (header);

    area.removeFromTop (18); // panel top padding
    auto inner = area.reduced (22, 20);

    auto loadRow = inner.removeFromTop (40);
    loadButton.setBounds (loadRow.removeFromLeft (150));
    loadRow.removeFromLeft (12);
    inputFileLabel.setBounds (loadRow);

    inner.removeFromTop (18);

    targetSectionLabel.setBounds (inner.removeFromTop (18));
    inner.removeFromTop (6);
    targetLoud.setBounds (inner.removeFromTop (26));
    targetNormal.setBounds (inner.removeFromTop (26));
    targetQuiet.setBounds (inner.removeFromTop (26));

    inner.removeFromTop (18);

    qualitySectionLabel.setBounds (inner.removeFromTop (18));
    inner.removeFromTop (6);
    qualityLow.setBounds (inner.removeFromTop (26));
    qualityMedium.setBounds (inner.removeFromTop (26));
    qualityHigh.setBounds (inner.removeFromTop (26));

    inner.removeFromTop (20);
    processButton.setBounds (inner.removeFromTop (42).removeFromLeft (220));

    inner.removeFromTop (14);
    statusLabel.setBounds (inner.removeFromTop (20));

    inner.removeFromTop (8);
    resultsLabel.setBounds (inner.removeFromTop (190));

    creditLabel.setBounds (getLocalBounds().removeFromBottom (26));
}

void MainComponent::chooseInputFile()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Select a finished WAV to simulate...",
        juce::File::getSpecialLocation (juce::File::userMusicDirectory),
        "*.wav;*.aiff;*.aif;*.flac");

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            inputFile = file;
            inputFileLabel.setText (file.getFileName(), juce::dontSendNotification);
            resultsLabel.setText ("Ready. Press \"Process & Save As...\" to continue.",
                                   juce::dontSendNotification);
        }
    });
}

void MainComponent::runProcessing()
{
    if (! inputFile.existsAsFile())
    {
        statusLabel.setText ("Choose an input WAV first.", juce::dontSendNotification);
        return;
    }
    if (isProcessing.load())
        return;

    fileChooser = std::make_unique<juce::FileChooser> (
        "Save the Spotify-simulated WAV as...",
        inputFile.getParentDirectory().getChildFile (inputFile.getFileNameWithoutExtension() + "_spotify_sim.wav"),
        "*.wav");

    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
    {
        auto outFile = fc.getResult();
        if (outFile == juce::File {})
            return;

        SpotifyProcessor::Target target = targetLoud.getToggleState()  ? SpotifyProcessor::Target::Loud
                                         : targetQuiet.getToggleState() ? SpotifyProcessor::Target::Quiet
                                         : SpotifyProcessor::Target::Normal;

        SpotifyProcessor::Quality quality = qualityLow.getToggleState()    ? SpotifyProcessor::Quality::Low96
                                           : qualityMedium.getToggleState() ? SpotifyProcessor::Quality::Medium160
                                           : SpotifyProcessor::Quality::High320;

        isProcessing = true;
        processButton.setEnabled (false);
        statusLabel.setText ("Processing...", juce::dontSendNotification);

        auto inFile = inputFile;
        std::thread worker ([this, inFile, outFile, target, quality]
        {
            SpotifyProcessor processor;
            auto report = processor.process (inFile, outFile, target, quality);

            juce::MessageManager::callAsync ([this, report]
            {
                isProcessing = false;
                processButton.setEnabled (true);
                showReport (report);
            });
        });
        worker.detach();
    });
}

juce::String MainComponent::formatResultsText (const SpotifyProcessor::Report& r)
{
    if (! r.success)
        return "Failed: " + r.errorMessage;

    juce::String s;
    s << "Target:            " << juce::String (r.targetLUFS, 1) << " LUFS\n"
      << "Applied gain:      " << (r.appliedGainDb >= 0 ? "+" : "") << juce::String (r.appliedGainDb, 2) << " dB\n\n"
      << "                     LUFS       True Peak\n"
      << "Original:          " << juce::String (r.inputLUFS, 1).paddedLeft (' ', 6)
                                << "     " << juce::String (r.inputTruePeak, 1) << " dBTP\n"
      << "After gain match:  " << juce::String (r.afterGainLUFS, 1).paddedLeft (' ', 6)
                                << "     " << juce::String (r.afterGainTruePeak, 1) << " dBTP\n"
      << "After Vorbis codec:" << juce::String (r.outputLUFS, 1).paddedLeft (' ', 6)
                                << "     " << juce::String (r.outputTruePeak, 1) << " dBTP\n\n"
      << "This WAV reflects the real quality loss from an actual lossy\n"
      << "encode/decode round trip, matching what Spotify's app plays.";
    return s;
}

void MainComponent::showReport (const SpotifyProcessor::Report& report)
{
    resultsLabel.setText (formatResultsText (report), juce::dontSendNotification);
    statusLabel.setText (report.success ? "Done." : "Error.", juce::dontSendNotification);
}

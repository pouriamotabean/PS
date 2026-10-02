#include "MainComponent.h"
#include "PSTheme.h"

MainComponent::MainComponent()
    : targetControl ({ { "Loud", "-11 LUFS" }, { "Normal", "-14 LUFS" }, { "Quiet", "-19 LUFS" } }),
      qualityControl ({ { "Low", "96 kbps" }, { "Medium", "160 kbps" }, { "High", "320 kbps" } })
{
    setLookAndFeel (&lookAndFeel);
    setSize (640, 800);

    titleLabel.setText ("PS", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (36.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, PSColours::text);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("Predict Spotify", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (15.0f)));
    subtitleLabel.setColour (juce::Label::textColourId, PSColours::textDim);
    addAndMakeVisible (subtitleLabel);

    loadButton.onClick = [this] { chooseInputFile(); };
    addAndMakeVisible (loadButton);

    inputFileLabel.setColour (juce::Label::textColourId, PSColours::textDim);
    inputFileLabel.setJustificationType (juce::Justification::centredLeft);
    inputFileLabel.setFont (juce::Font (juce::FontOptions (14.0f)));
    addAndMakeVisible (inputFileLabel);

    auto setupSectionLabel = [] (juce::Label& l)
    {
        l.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
        l.setColour (juce::Label::textColourId, PSColours::gold);
    };
    setupSectionLabel (targetSectionLabel);
    setupSectionLabel (qualitySectionLabel);
    addAndMakeVisible (targetSectionLabel);
    addAndMakeVisible (qualitySectionLabel);

    targetControl.setSelectedIndex (1, juce::dontSendNotification);  // Normal
    qualityControl.setSelectedIndex (2, juce::dontSendNotification); // High
    addAndMakeVisible (targetControl);
    addAndMakeVisible (qualityControl);

    processButton.onClick = [this] { runProcessing(); };
    processButton.setColour (juce::TextButton::buttonColourId, PSColours::accent);
    processButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible (processButton);

    addAndMakeVisible (resultsPanel);

    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    statusLabel.setColour (juce::Label::textColourId, PSColours::accentHi);
    addAndMakeVisible (statusLabel);

    creditLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    creditLabel.setColour (juce::Label::textColourId, PSColours::textDim);
    creditLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (creditLabel);
}

MainComponent::~MainComponent()
{
    setLookAndFeel (nullptr);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (PSColours::bg);

    auto panelBounds = getLocalBounds().reduced (18).withTrimmedTop (88).toFloat();
    g.setColour (PSColours::panel);
    g.fillRoundedRectangle (panelBounds, 14.0f);
    g.setColour (PSColours::border);
    g.drawRoundedRectangle (panelBounds, 14.0f, 1.0f);

    // Small accent mark next to the title, echoing the PQ/PD/PV family mark.
    auto markBounds = juce::Rectangle<float> (18.0f, 26.0f, 8.0f, 34.0f);
    juce::ColourGradient grad (PSColours::accentHi, markBounds.getX(), markBounds.getY(),
                                PSColours::accent, markBounds.getX(), markBounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (markBounds, 3.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (70);
    header.removeFromLeft (20); // room for the accent mark drawn in paint()
    titleLabel.setBounds (header.removeFromTop (44));
    subtitleLabel.setBounds (header);

    area.removeFromTop (18); // panel top padding
    auto inner = area.reduced (22, 20);

    auto loadRow = inner.removeFromTop (42);
    loadButton.setBounds (loadRow.removeFromLeft (150));
    loadRow.removeFromLeft (14);
    inputFileLabel.setBounds (loadRow);

    inner.removeFromTop (22);

    targetSectionLabel.setBounds (inner.removeFromTop (16));
    inner.removeFromTop (8);
    targetControl.setBounds (inner.removeFromTop (56));

    inner.removeFromTop (22);

    qualitySectionLabel.setBounds (inner.removeFromTop (16));
    inner.removeFromTop (8);
    qualityControl.setBounds (inner.removeFromTop (56));

    inner.removeFromTop (26);
    processButton.setBounds (inner.removeFromTop (46));

    inner.removeFromTop (14);
    statusLabel.setBounds (inner.removeFromTop (18));

    inner.removeFromTop (8);
    resultsPanel.setBounds (inner.removeFromTop (200));

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
            resultsPanel.setPlaceholder ("Ready. Press \"Process & Save As...\" to continue.");
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

        auto target  = (SpotifyProcessor::Target)  targetControl.getSelectedIndex();
        auto quality = (SpotifyProcessor::Quality) qualityControl.getSelectedIndex();

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

void MainComponent::showReport (const SpotifyProcessor::Report& report)
{
    resultsPanel.setReport (report);
    statusLabel.setText (report.success ? "Done." : "Error.", juce::dontSendNotification);
}

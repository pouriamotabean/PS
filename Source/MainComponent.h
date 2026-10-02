#pragma once
#include <JuceHeader.h>
#include "SpotifyProcessor.h"
#include "SegmentedControl.h"
#include "ResultsPanel.h"
#include "PSLookAndFeel.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void chooseInputFile();
    void runProcessing();
    void showReport (const SpotifyProcessor::Report& report);

    PSLookAndFeel lookAndFeel;

    // --- Header -------------------------------------------------------
    juce::Label titleLabel, subtitleLabel;

    // --- Input ----------------------------------------------------------
    juce::TextButton loadButton { "Choose WAV..." };
    juce::Label inputFileLabel { {}, "No file selected" };

    // --- Target loudness ------------------------------------------------
    juce::Label targetSectionLabel { {}, "SPOTIFY LOUDNESS TARGET" };
    SegmentedControl targetControl;

    // --- Quality tier -----------------------------------------------------
    juce::Label qualitySectionLabel { {}, "STREAMING QUALITY" };
    SegmentedControl qualityControl;

    // --- Action / results -------------------------------------------------
    juce::TextButton processButton { "Process & Save As..." };
    ResultsPanel resultsPanel;
    juce::Label statusLabel;
    juce::Label creditLabel { {}, "by Pouria Motabean" };

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File inputFile;
    std::atomic<bool> isProcessing { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

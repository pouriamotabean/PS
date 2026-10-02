#pragma once
#include <JuceHeader.h>
#include "SpotifyProcessor.h"

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

    static juce::String formatResultsText (const SpotifyProcessor::Report& r);

    // --- Header -------------------------------------------------------
    juce::Label titleLabel, subtitleLabel;

    // --- Input ----------------------------------------------------------
    juce::TextButton loadButton { "Choose WAV..." };
    juce::Label inputFileLabel { {}, "No file selected" };

    // --- Target loudness ------------------------------------------------
    juce::Label targetSectionLabel { {}, "SPOTIFY LOUDNESS TARGET" };
    juce::ToggleButton targetLoud   { "Loud  (-11 LUFS)" };
    juce::ToggleButton targetNormal { "Normal  (-14 LUFS)" };
    juce::ToggleButton targetQuiet  { "Quiet  (-19 LUFS)" };

    // --- Quality tier -----------------------------------------------------
    juce::Label qualitySectionLabel { {}, "STREAMING QUALITY" };
    juce::ToggleButton qualityLow    { "Low  (96 kbps - mobile data saver)" };
    juce::ToggleButton qualityMedium { "Medium  (160 kbps)" };
    juce::ToggleButton qualityHigh   { "High  (320 kbps - Premium WiFi)" };

    // --- Action / results -------------------------------------------------
    juce::TextButton processButton { "Process && Save As..." };
    juce::Label resultsLabel;
    juce::Label statusLabel;
    juce::Label creditLabel { {}, "PS  /  SPOTIFY PLAYBACK SIMULATOR   /   Pouria Motabean" };

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File inputFile;
    std::atomic<bool> isProcessing { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

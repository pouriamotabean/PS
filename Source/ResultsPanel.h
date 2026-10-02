#pragma once
#include <JuceHeader.h>
#include "SpotifyProcessor.h"

// A clean, aligned results card: target + applied gain up top, then a
// three-row LUFS / True-Peak comparison table (Original -> after gain
// match -> after the real codec round trip).
class ResultsPanel : public juce::Component
{
public:
    ResultsPanel();

    void setPlaceholder (const juce::String& text);
    void setReport (const SpotifyProcessor::Report& report);

    void paint (juce::Graphics& g) override;

private:
    bool hasReport = false;
    bool isError = false;
    juce::String placeholderText { "Load a WAV and press Process to see results here." };
    SpotifyProcessor::Report report;

    void drawRow (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& label,
                  const juce::String& lufs, const juce::String& tp, bool bold, juce::Colour labelColour) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResultsPanel)
};

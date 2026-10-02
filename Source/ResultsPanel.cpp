#include "ResultsPanel.h"
#include "PSTheme.h"

ResultsPanel::ResultsPanel() = default;

void ResultsPanel::setPlaceholder (const juce::String& text)
{
    hasReport = false;
    isError = false;
    placeholderText = text;
    repaint();
}

void ResultsPanel::setReport (const SpotifyProcessor::Report& r)
{
    report = r;
    hasReport = true;
    isError = ! r.success;
    repaint();
}

void ResultsPanel::drawRow (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& label,
                             const juce::String& lufs, const juce::String& tp, bool bold, juce::Colour labelColour) const
{
    auto labelArea = area.removeFromLeft (juce::roundToInt ((float) area.getWidth() * 0.46f));
    auto lufsArea  = area.removeFromLeft (juce::roundToInt ((float) area.getWidth() * 0.55f));
    auto tpArea    = area;

    g.setFont (juce::Font (juce::FontOptions (13.0f, bold ? juce::Font::bold : juce::Font::plain)));
    g.setColour (labelColour);
    g.drawText (label, labelArea, juce::Justification::centredLeft);

    g.setColour (bold ? PSColours::text : PSColours::text.withAlpha (0.92f));
    g.drawText (lufs, lufsArea, juce::Justification::centredLeft);
    g.drawText (tp,   tpArea,   juce::Justification::centredLeft);
}

void ResultsPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (PSColours::raised);
    g.fillRoundedRectangle (bounds, 10.0f);
    g.setColour (PSColours::border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 10.0f, 1.0f);

    auto area = getLocalBounds().reduced (18, 14);

    if (! hasReport)
    {
        g.setFont (juce::Font (juce::FontOptions (13.5f)));
        g.setColour (PSColours::textDim);
        g.drawFittedText (placeholderText, area, juce::Justification::centredLeft, 3);
        return;
    }

    if (isError)
    {
        g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
        g.setColour (PSColours::bad);
        g.drawFittedText ("Something went wrong:\n" + report.errorMessage, area, juce::Justification::centredLeft, 4);
        return;
    }

    // --- Header: target + applied gain ---------------------------------
    auto top = area.removeFromTop (26);
    g.setFont (juce::Font (juce::FontOptions (13.5f, juce::Font::bold)));
    g.setColour (PSColours::gold);
    g.drawText ("TARGET  " + juce::String (report.targetLUFS, 1) + " LUFS",
                top.removeFromLeft (area.getWidth() / 2), juce::Justification::centredLeft);

    auto gainColour = report.appliedGainDb > 0.01  ? PSColours::good
                     : report.appliedGainDb < -0.01 ? PSColours::warn
                                                      : PSColours::textDim;
    juce::String gainText = (report.appliedGainDb >= 0 ? "+" : "") + juce::String (report.appliedGainDb, 2) + " dB applied";
    g.setFont (juce::Font (juce::FontOptions (13.5f, juce::Font::bold)));
    g.setColour (gainColour);
    g.drawText (gainText, top, juce::Justification::centredRight);

    area.removeFromTop (10);
    g.setColour (PSColours::border);
    g.drawLine ((float) area.getX(), (float) area.getY(), (float) area.getRight(), (float) area.getY(), 1.0f);
    area.removeFromTop (10);

    // --- Column headers ---------------------------------------------------
    drawRow (g, area.removeFromTop (18), "", "LUFS", "TRUE PEAK", true, PSColours::textDim);
    area.removeFromTop (4);

    const int rowH = 24;
    drawRow (g, area.removeFromTop (rowH), "Original",
             juce::String (report.inputLUFS, 1), juce::String (report.inputTruePeak, 1) + " dBTP",
             false, PSColours::textDim);
    drawRow (g, area.removeFromTop (rowH), "After gain match",
             juce::String (report.afterGainLUFS, 1), juce::String (report.afterGainTruePeak, 1) + " dBTP",
             false, PSColours::textDim);
    drawRow (g, area.removeFromTop (rowH), "After Vorbis codec",
             juce::String (report.outputLUFS, 1), juce::String (report.outputTruePeak, 1) + " dBTP",
             true, PSColours::accentHi);

    area.removeFromTop (10);
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    g.setColour (PSColours::textDim);
    g.drawFittedText (
        "This WAV reflects the real quality loss from an actual lossy encode/decode round trip, matching what Spotify's app plays.",
        area, juce::Justification::topLeft, 2);
}

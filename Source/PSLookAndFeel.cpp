#include "PSLookAndFeel.h"
#include "PSTheme.h"

PSLookAndFeel::PSLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, PSColours::bg);
    setColour (juce::TextButton::buttonColourId, PSColours::raised);
    setColour (juce::TextButton::textColourOffId, PSColours::text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

juce::Font PSLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jmin (16.0f, (float) buttonHeight * 0.42f), juce::Font::bold));
}

void PSLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                           bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const float radius = juce::jmin (10.0f, bounds.getHeight() * 0.3f);

    auto base = backgroundColour;
    if (isDown)              base = base.darker (0.25f);
    else if (isHighlighted)  base = base.brighter (0.10f);

    juce::ColourGradient grad (base.brighter (0.14f), bounds.getX(), bounds.getY(),
                                base.darker (0.10f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);

    g.setColour (juce::Colours::black.withAlpha (0.30f));
    g.drawLine (bounds.getX() + radius * 0.3f, bounds.getBottom() - 0.5f,
                bounds.getRight() - radius * 0.3f, bounds.getBottom() - 0.5f, 1.0f);
}

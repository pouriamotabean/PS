#pragma once
#include <JuceHeader.h>
#include "LoudnessMeter.h"

// Reproduces what Spotify actually does to a master before you hear it:
// 1. Loudness-normalize to one of Spotify's published targets, with the
//    same true-peak safety cap Spotify uses when boosting a quiet track.
// 2. Run the result through a REAL Ogg Vorbis encode -> decode round trip
//    at the chosen bitrate tier, so the codec's actual quality loss is
//    audible, not simulated.
// The output is handed back as plain PCM (WAV) — the "natural" form the
// user asked for, i.e. exactly what Spotify's own decoder would hand to
// your audio driver.
class SpotifyProcessor
{
public:
    enum class Target { Loud, Normal, Quiet };
    enum class Quality { Low96, Medium160, High320 };

    struct Report
    {
        bool success = false;
        juce::String errorMessage;

        double inputLUFS = 0.0, inputTruePeak = 0.0;
        double afterGainLUFS = 0.0, afterGainTruePeak = 0.0;
        double outputLUFS = 0.0, outputTruePeak = 0.0;
        double appliedGainDb = 0.0;
        double targetLUFS = 0.0;
    };

    static double targetToLUFS (Target t)
    {
        switch (t)
        {
            case Target::Loud:   return -11.0;
            case Target::Quiet:  return -19.0;
            case Target::Normal: default: return -14.0;
        }
    }

    // Spotify's documented ceiling: when boosting a quiet track, it will
    // not push the true peak past roughly -1 dBTP, to stay clip-safe
    // through downstream lossy decoding.
    static constexpr double kMaxTruePeakCeilingDbTP = -1.0;

    static int qualityToKbps (Quality q)
    {
        switch (q)
        {
            case Quality::Low96:    return 96;
            case Quality::Medium160: return 160;
            case Quality::High320:  default: return 320;
        }
    }

    // Runs the whole pipeline. inputFile must be a WAV/AIFF/FLAC Juce can
    // read. outputFile will be written as a 24-bit WAV. Returns a Report.
    Report process (const juce::File& inputFile, const juce::File& outputFile,
                     Target target, Quality quality);

private:
    static int qualityToOggIndex (Quality q, int numAvailableOptions);
};

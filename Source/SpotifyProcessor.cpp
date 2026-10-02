#include "SpotifyProcessor.h"

int SpotifyProcessor::qualityToOggIndex (Quality q, int numAvailableOptions)
{
    const int wantedKbps = qualityToKbps (q);

    juce::OggVorbisAudioFormat ogg;
    juce::StringArray options = ogg.getQualityOptions();

    int bestIndex = 0;
    int bestDiff = std::numeric_limits<int>::max();
    bool foundNumeric = false;

    for (int i = 0; i < options.size(); ++i)
    {
        juce::String s = options[i];
        juce::String digits;
        for (auto ch : s)
            if (juce::CharacterFunctions::isDigit (ch))
                digits += ch;
            else if (digits.isNotEmpty())
                break; // stop at first run of digits (e.g. "96 kbps")

        if (digits.isNotEmpty())
        {
            int kbps = digits.getIntValue();
            int diff = std::abs (kbps - wantedKbps);
            if (diff < bestDiff)
            {
                bestDiff = diff;
                bestIndex = i;
                foundNumeric = true;
            }
        }
    }

    if (foundNumeric)
        return bestIndex;

    // Fallback: no numeric labels available from this JUCE build — spread
    // our three tiers evenly across whatever options exist.
    const int n = juce::jmax (1, numAvailableOptions);
    const double t = juce::jlimit (0.0, 1.0, (double) (wantedKbps - 96) / (double) (320 - 96));
    return juce::jlimit (0, n - 1, (int) std::round (t * (double) (n - 1)));
}

SpotifyProcessor::Report SpotifyProcessor::process (const juce::File& inputFile, const juce::File& outputFile,
                                                      Target target, Quality quality)
{
    Report report;
    report.targetLUFS = targetToLUFS (target);

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (inputFile));
    if (reader == nullptr)
    {
        report.errorMessage = "Could not read input file: " + inputFile.getFullPathName();
        return report;
    }

    const double sampleRate = reader->sampleRate;
    const int numChannels = (int) reader->numChannels;
    const int numSamples = (int) reader->lengthInSamples;

    if (numSamples <= 0 || numChannels <= 0)
    {
        report.errorMessage = "Input file has no audio data.";
        return report;
    }

    juce::AudioBuffer<float> buffer (numChannels, numSamples);
    reader->read (&buffer, 0, numSamples, 0, true, true);
    reader.reset();

    // --- Step 1: measure input loudness -----------------------------
    auto inMeasure = LoudnessMeter::measure (buffer, sampleRate);
    report.inputLUFS = inMeasure.integratedLUFS;
    report.inputTruePeak = inMeasure.truePeakDbTP;

    // --- Step 2: Spotify-style gain match ---------------------------
    const double diff = report.targetLUFS - report.inputLUFS;
    double appliedGain;
    if (diff <= 0.0)
    {
        appliedGain = diff; // louder than target: always attenuate fully
    }
    else
    {
        const double maxGainByTruePeak = kMaxTruePeakCeilingDbTP - report.inputTruePeak;
        appliedGain = juce::jmax (0.0, juce::jmin (diff, maxGainByTruePeak));
    }
    report.appliedGainDb = appliedGain;

    const float linearGain = (float) std::pow (10.0, appliedGain / 20.0);
    buffer.applyGain (linearGain);

    // Gain shifts LUFS and true peak by exactly the same dB amount.
    report.afterGainLUFS = report.inputLUFS + appliedGain;
    report.afterGainTruePeak = report.inputTruePeak + appliedGain;

    // Safety: never let sample peaks exceed 0 dBFS going into the encoder.
    float samplePeak = buffer.getMagnitude (0, numSamples);
    if (samplePeak > 1.0f)
        buffer.applyGain (1.0f / samplePeak);

    // --- Step 3: real Ogg Vorbis encode -> decode round trip --------
    juce::File tempOgg = juce::File::createTempFile ("ps_spotify_sim.ogg");
    juce::OggVorbisAudioFormat oggFormat;

    {
        std::unique_ptr<juce::OutputStream> os (tempOgg.createOutputStream());
        if (os == nullptr)
        {
            report.errorMessage = "Could not create temporary Ogg file.";
            return report;
        }

        const int qualityIndex = qualityToOggIndex (quality, oggFormat.getQualityOptions().size());

        auto options = juce::AudioFormatWriterOptions()
                           .withSampleRate (sampleRate)
                           .withNumChannels (numChannels)
                           .withBitsPerSample (32)
                           .withQualityOptionIndex (qualityIndex);

        std::unique_ptr<juce::AudioFormatWriter> writer (oggFormat.createWriterFor (os, options));
        if (writer == nullptr)
        {
            report.errorMessage = "Could not create Ogg Vorbis encoder.";
            return report;
        }

        writer->writeFromAudioSampleBuffer (buffer, 0, numSamples);
        writer.reset(); // flush + close
    }

    std::unique_ptr<juce::AudioFormatReader> oggReader (oggFormat.createReaderFor (
        new juce::FileInputStream (tempOgg), true));
    if (oggReader == nullptr)
    {
        report.errorMessage = "Could not decode the Ogg Vorbis round-trip file.";
        tempOgg.deleteFile();
        return report;
    }

    const int decodedChannels = (int) oggReader->numChannels;
    const int decodedSamples = (int) oggReader->lengthInSamples;
    juce::AudioBuffer<float> decoded (decodedChannels, decodedSamples);
    oggReader->read (&decoded, 0, decodedSamples, 0, true, true);
    oggReader.reset();
    tempOgg.deleteFile();

    // --- Step 4: measure the real post-codec result ------------------
    auto outMeasure = LoudnessMeter::measure (decoded, sampleRate);
    report.outputLUFS = outMeasure.integratedLUFS;
    report.outputTruePeak = outMeasure.truePeakDbTP;

    // --- Step 5: write the natural WAV output -------------------------
    outputFile.deleteFile();
    std::unique_ptr<juce::OutputStream> wavStream (outputFile.createOutputStream());
    if (wavStream == nullptr)
    {
        report.errorMessage = "Could not create output WAV file: " + outputFile.getFullPathName();
        return report;
    }

    juce::WavAudioFormat wavFormat;
    auto wavOptions = juce::AudioFormatWriterOptions()
                           .withSampleRate (sampleRate)
                           .withNumChannels (decodedChannels)
                           .withBitsPerSample (24);

    std::unique_ptr<juce::AudioFormatWriter> wavWriter (wavFormat.createWriterFor (wavStream, wavOptions));
    if (wavWriter == nullptr)
    {
        report.errorMessage = "Could not create WAV writer.";
        return report;
    }

    wavWriter->writeFromAudioSampleBuffer (decoded, 0, decodedSamples);
    wavWriter.reset();

    report.success = true;
    return report;
}

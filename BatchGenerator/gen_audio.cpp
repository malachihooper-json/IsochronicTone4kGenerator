#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <random>
#include <string>
#include <vector>

// --- Organic Noise Synthesizer ---
class OrganicNoiseSynth {
public:
  enum Type { Brown, Pink, White };

  void prepare(double sampleRateHz, Type noiseType,
               float targetEntrainmentFreq) {
    this->sampleRate = sampleRateHz;
    this->type = noiseType;
    lastOut = 0.0f;
    for (int i = 0; i < 7; ++i)
      pinkRows[i] = 0.0f;

    // Contextual LFO: Higher frequencies get faster "shimming" wind
    // Lower frequencies get slow, heavy wave swells
    float lfoHz = (targetEntrainmentFreq < 8.0f) ? 0.05f : 0.2f;
    lfoPhase = 0.0;
    lfoIncr =
        (2.0 * juce::MathConstants<double>::pi * (double)lfoHz) / sampleRate;
  }

  float process() {
    float raw = 0.0f;
    if (type == White) {
      raw = static_cast<float>(dist(gen));
    } else if (type == Brown) {
      float r = static_cast<float>(dist(gen));
      lastOut = (lastOut + (0.05f * r)) / 1.01f;
      raw = lastOut * 3.5f;
    } else if (type == Pink) {
      float r = static_cast<float>(dist(gen));
      float sum = 0.0f;
      for (int i = 0; i < 7; ++i) {
        if ((pinkCount & (1 << i)) != 0)
          pinkRows[i] = r;
        sum += pinkRows[i];
      }
      pinkCount++;
      raw = sum * 0.12f;
    }

    float lfo = static_cast<float>((std::sin(lfoPhase) + 1.0) * 0.5);
    lfoPhase += lfoIncr;
    if (lfoPhase > 2.0 * juce::MathConstants<double>::pi)
      lfoPhase -= 2.0 * juce::MathConstants<double>::pi;

    // Dynamic Filter: Sweeps the "air" or "depth" of the noise
    float alpha = 0.005f + (lfo * 0.05f);
    filterLast = filterLast + alpha * (raw - filterLast);
    return filterLast;
  }

private:
  double sampleRate = 44100.0;
  Type type = White;
  float lastOut = 0.0f;
  float pinkRows[7] = {0};
  int pinkCount = 0;
  float filterLast = 0.0f;
  double lfoPhase = 0.0, lfoIncr = 0.0;
  std::mt19937 gen{std::random_device{}()};
  std::uniform_real_distribution<float> dist{-1.0f, 1.0f};
};

// --- Mastering Chain ---
class SimpleReverb {
public:
  void prepare(double sampleRateHz) {
    float combTimes[] = {0.0297f, 0.0371f, 0.0411f, 0.0437f};
    combs.clear();
    for (float t : combTimes)
      combs.emplace_back(static_cast<int>(t * sampleRateHz));
    allPass.prepare(static_cast<int>(0.005f * sampleRateHz), 0.7f);
  }
  float process(float input) {
    if (combs.empty())
      return input;
    float combined = 0.0f;
    for (auto &c : combs)
      combined += c.process(input);
    return allPass.process(combined * 0.25f);
  }

private:
  struct Comb {
    std::vector<float> buffer;
    int idx = 0;
    float feedback = 0.84f;
    Comb(int size) : buffer(std::max(1, size), 0.0f) {}
    float process(float in) {
      float out = buffer[static_cast<size_t>(idx)];
      buffer[static_cast<size_t>(idx)] = in + (out * feedback);
      if (++idx >= static_cast<int>(buffer.size()))
        idx = 0;
      return out;
    }
  };
  struct AllPass {
    std::vector<float> buffer;
    int idx = 0;
    float g = 0.5f;
    void prepare(int size, float gain) {
      if (size > 0)
        buffer.assign(static_cast<size_t>(size), 0.0f);
      g = gain;
    }
    float process(float in) {
      if (buffer.empty())
        return in;
      float v = buffer[static_cast<size_t>(idx)];
      float out = -g * in + v;
      buffer[static_cast<size_t>(idx)] = in + g * out;
      if (++idx >= static_cast<int>(buffer.size()))
        idx = 0;
      return out;
    }
  };
  std::vector<Comb> combs;
  AllPass allPass;
};

struct JourneyPoint {
  double timeSec;
  float pulseFreq;
  float carrierFreq;
};

std::vector<JourneyPoint> parseJourney(const std::string &pulseStr,
                                       const std::string &carrierStr,
                                       double duration) {
  auto split = [](const std::string &s) {
    std::vector<float> v;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ','))
      v.push_back(std::stof(item));
    return v;
  };

  std::vector<float> pulses = split(pulseStr);
  std::vector<float> carriers = split(carrierStr);
  std::vector<JourneyPoint> points;

  // We expect exactly 5 points for a "Quarter Point" journey
  if (pulses.size() != 5 || carriers.size() != 5)
    return points;

  double milestones[5] = {0.0, 0.25, 0.5, 0.75, 1.0};
  for (int i = 0; i < 5; ++i) {
    points.push_back({milestones[i] * duration, pulses[i], carriers[i]});
  }
  return points;
}

int main(int argc, char *argv[]) {
  if (argc < 7) {
    std::cout << "Usage: IsochronicBatchGen <output_wav> <duration_seconds> "
                 "<pulse_freq_or_points> <carrier_freq_or_points> <softness> "
                 "<type> [gain_db] [noise_type] [noise_level]"
              << std::endl;
    return 1;
  }

  try {
    juce::File outputFile(argv[1]);
    double durationSeconds = std::stod(argv[2]);
    std::string pulseArg = argv[3];
    std::string carrierArg = argv[4];
    float softness = std::stof(argv[5]);
    int entrainmentType = std::stoi(argv[6]);
    float gainDb = (argc >= 8) ? std::stof(argv[7]) : -10.0f;
    float gain = juce::Decibels::decibelsToGain(gainDb);
    int noiseTypeIdx = (argc >= 9) ? std::stoi(argv[8]) : -1;
    float noiseLevel = (argc >= 10) ? std::stof(argv[9]) : 0.3f;

    bool isJourney = (pulseArg.find(',') != std::string::npos);
    std::vector<JourneyPoint> journeyPoints;
    if (isJourney) {
      journeyPoints = parseJourney(pulseArg, carrierArg, durationSeconds);
      if (journeyPoints.empty()) {
        std::cerr << "Error: Invalid journey points (expected 5 values)."
                  << std::endl;
        return 1;
      }
    }

    double sampleRate = 44100.0;
    int64_t totalSamples = static_cast<int64_t>(durationSeconds * sampleRate);

    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (!stream) {
      std::cerr << "Error: Could not create output stream" << std::endl;
      return 1;
    }

    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(stream.release(), sampleRate, 2, 24, {}, 0));
    if (!writer) {
      std::cerr << "Error: Could not create WAV writer" << std::endl;
      return 1;
    }

    SimpleReverb reverbL, reverbR;
    reverbL.prepare(sampleRate);
    reverbR.prepare(sampleRate);

    OrganicNoiseSynth noiseL, noiseR;
    if (noiseTypeIdx >= 0) {
      float basePulse = isJourney ? journeyPoints[0].pulseFreq : std::stof(pulseArg);
      noiseL.prepare(sampleRate, static_cast<OrganicNoiseSynth::Type>(noiseTypeIdx), basePulse);
      noiseR.prepare(sampleRate, static_cast<OrganicNoiseSynth::Type>(noiseTypeIdx), basePulse);
    }

    const int blockSize = 8192;
    juce::AudioBuffer<float> buffer(2, blockSize);
    double carrierPhase = 0.0, pulsePhase = 0.0, phaseL = 0.0, phaseR = 0.0;
    const double pi2 = 2.0 * juce::MathConstants<double>::pi;
    const float softnessExp = 1.0f + (softness * 4.0f);

    int64_t samplesProcessed = 0;
    while (samplesProcessed < totalSamples) {
      int64_t samplesThisBlock = std::min(static_cast<int64_t>(blockSize),
                                          totalSamples - samplesProcessed);
      buffer.clear();
      auto *dataL = buffer.getWritePointer(0);
      auto *dataR = buffer.getWritePointer(1);

      for (int i = 0; i < static_cast<int>(samplesThisBlock); ++i) {
        double currentTime = (double)(samplesProcessed + i) / sampleRate;
        float currentPulse = 0.0f;
        float currentCarrier = 0.0f;

        if (isJourney) {
          // Linear interpolation for journey
          int idx = 0;
          while (idx < 4 && currentTime > journeyPoints[idx + 1].timeSec)
            idx++;
          double t1 = journeyPoints[idx].timeSec;
          double t2 = journeyPoints[idx + 1].timeSec;
          double alpha = (currentTime - t1) / (t2 - t1);
          currentPulse = journeyPoints[idx].pulseFreq +
                         alpha * (journeyPoints[idx + 1].pulseFreq -
                                  journeyPoints[idx].pulseFreq);
          currentCarrier = journeyPoints[idx].carrierFreq +
                           alpha * (journeyPoints[idx + 1].carrierFreq -
                                    journeyPoints[idx].carrierFreq);
        } else {
          currentPulse = std::stof(pulseArg);
          currentCarrier = std::stof(carrierArg);
        }

        const double carrierIncr = (pi2 * (double)currentCarrier) / sampleRate;
        const double pulseIncr = (pi2 * (double)currentPulse) / sampleRate;
        const double incrL = (pi2 * ((double)currentCarrier - ((double)currentPulse / 2.0))) / sampleRate;
        const double incrR = (pi2 * ((double)currentCarrier + ((double)currentPulse / 2.0))) / sampleRate;

        float outL = 0.0f, outR = 0.0f;
        if (entrainmentType == 0) {
          float c = std::sin(static_cast<float>(carrierPhase));
          carrierPhase += carrierIncr;
          float p = std::pow(std::max(0.0f, std::sin(static_cast<float>(pulsePhase))), softnessExp);
          pulsePhase += pulseIncr;
          outL = c * p * gain;
          outR = outL;
        } else {
          outL = std::sin(static_cast<float>(phaseL)) * gain;
          phaseL += incrL;
          outR = std::sin(static_cast<float>(phaseR)) * gain;
          phaseR += incrR;
        }

        if (carrierPhase > pi2) carrierPhase -= pi2;
        if (pulsePhase > pi2) pulsePhase -= pi2;
        if (phaseL > pi2) phaseL -= pi2;
        if (phaseR > pi2) phaseR -= pi2;

        if (noiseTypeIdx >= 0) {
          outL += noiseL.process() * noiseLevel;
          outR += noiseR.process() * noiseLevel;
        }

        float wetL = reverbL.process(outL);
        float wetR = reverbR.process(outR);
        dataL[i] = std::tanh(outL + wetL * 0.12f);
        dataR[i] = std::tanh(outR + wetR * 0.12f);
      }
      writer->writeFromAudioSampleBuffer(buffer, 0, static_cast<int>(samplesThisBlock));
      samplesProcessed += samplesThisBlock;
      if (samplesProcessed % (static_cast<int64_t>(sampleRate) * 5) == 0) {
        std::cout << "\rProgress: " << std::fixed << std::setprecision(1)
                  << (100.0 * (double)samplesProcessed / (double)totalSamples)
                  << "%" << std::flush;
      }
    }
    std::cout << "\nGeneration Successful." << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "\nRuntime Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

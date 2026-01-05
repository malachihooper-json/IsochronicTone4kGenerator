#include "PluginProcessor.h"
#include "PluginEditor.h"

IsochronicToneGenAudioProcessor::IsochronicToneGenAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
#endif
      apvts(*this, nullptr, "Parameters", createParameterLayout()) {
}

IsochronicToneGenAudioProcessor::~IsochronicToneGenAudioProcessor() {}

const juce::String IsochronicToneGenAudioProcessor::getName() const {
  return JucePlugin_Name;
}
bool IsochronicToneGenAudioProcessor::acceptsMidi() const { return true; }
bool IsochronicToneGenAudioProcessor::producesMidi() const { return false; }
bool IsochronicToneGenAudioProcessor::isMidiEffect() const { return false; }
double IsochronicToneGenAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}
int IsochronicToneGenAudioProcessor::getNumPrograms() { return 1; }
int IsochronicToneGenAudioProcessor::getCurrentProgram() { return 0; }
void IsochronicToneGenAudioProcessor::setCurrentProgram(int index) {}
const juce::String IsochronicToneGenAudioProcessor::getProgramName(int index) {
  return {};
}
void IsochronicToneGenAudioProcessor::changeProgramName(
    int index, const juce::String &newName) {}

void IsochronicToneGenAudioProcessor::prepareToPlay(double sampleRate,
                                                    int samplesPerBlock) {
  currentSampleRate = sampleRate;
}

void IsochronicToneGenAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IsochronicToneGenAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;
  return true;
}
#endif

void IsochronicToneGenAudioProcessor::processBlock(
    juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  float carrierFreq = apvts.getRawParameterValue("CARRIER_FREQ")->load();
  float pulseFreq = apvts.getRawParameterValue("PULSE_FREQ")->load();
  float softness = apvts.getRawParameterValue("SOFTNESS")->load();
  float gain = juce::Decibels::decibelsToGain(
      apvts.getRawParameterValue("GAIN")->load());

  auto *channelDataL = buffer.getWritePointer(0);
  auto *channelDataR =
      totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

  for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
    // Basic sine wave for the carrier. We might want to swap this for
    // something richer later, but for now, it's clean.
    float carrierSample = std::sin(carrierPhase);
    carrierPhase += (2.0 * juce::MathConstants<double>::pi * carrierFreq) /
                    currentSampleRate;
    if (carrierPhase > 2.0 * juce::MathConstants<double>::pi)
      carrierPhase -= 2.0 * juce::MathConstants<double>::pi;

    // This is where the isochronic effect happens. We modulate the volume
    // using a sine-based pulse and then shape it.
    float pulseVal = std::sin(pulsePhase);

    // We smooth out the edges here. If the pulses are too sharp (like a
    // perfect square), they click and it sounds nasty.
    float smoothedPulse =
        std::pow(std::abs(pulseVal), 1.0f - (softness * 0.9f));

    // Only play audio during the positive half of the pulse LFO.
    if (pulseVal < 0)
      smoothedPulse = 0;

    pulsePhase +=
        (2.0 * juce::MathConstants<double>::pi * pulseFreq) / currentSampleRate;
    if (pulsePhase > 2.0 * juce::MathConstants<double>::pi)
      pulsePhase -= 2.0 * juce::MathConstants<double>::pi;

    float out = carrierSample * smoothedPulse * gain;

    // Quick soft clipper to prevent the output from slamming the 0dB ceiling.
    if (out > 0.9f)
      out = 0.9f + (out - 0.9f) * 0.1f;
    else if (out < -0.9f)
      out = -0.9f + (out + 0.9f) * 0.1f;

    channelDataL[sample] = out;
    if (channelDataR != nullptr)
      channelDataR[sample] = out;
  }
}

bool IsochronicToneGenAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor *IsochronicToneGenAudioProcessor::createEditor() {
  return new IsochronicToneGenEditor(*this);
}

void IsochronicToneGenAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void IsochronicToneGenAudioProcessor::setStateInformation(const void *data,
                                                          int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout
IsochronicToneGenAudioProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "PULSE_FREQ", "Entrainment Freq (Hz)", 0.5f, 60.0f, 10.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "CARRIER_FREQ", "Carrier Freq (Hz)", 40.0f, 1000.0f, 440.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "SOFTNESS", "Softness", 0.0f, 1.0f, 0.5f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "GAIN", "Volume (dB)", -60.0f, 0.0f, -6.0f));

  return {params.begin(), params.end()};
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new IsochronicToneGenAudioProcessor();
}

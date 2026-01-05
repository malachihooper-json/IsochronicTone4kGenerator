#pragma once

#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class IsochronicToneGenEditor : public juce::AudioProcessorEditor {
public:
  IsochronicToneGenEditor(IsochronicToneGenAudioProcessor &);
  ~IsochronicToneGenEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  IsochronicToneGenAudioProcessor &audioProcessor;

  juce::Slider pulseFreqSlider;
  juce::Slider carrierFreqSlider;
  juce::Slider softnessSlider;
  juce::Slider gainSlider;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      pulseFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      carrierFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      softnessAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      gainAttachment;

  juce::Label pulseFreqLabel;
  juce::Label carrierFreqLabel;
  juce::Label softnessLabel;
  juce::Label gainLabel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IsochronicToneGenEditor)
};

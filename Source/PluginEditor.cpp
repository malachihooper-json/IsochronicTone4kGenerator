#include "PluginEditor.h"
#include "PluginProcessor.h"

IsochronicToneGenEditor::IsochronicToneGenEditor(
    IsochronicToneGenAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(400, 300);

  auto setupSlider =
      [this](
          juce::Slider &slider, juce::Label &label, const juce::String &text,
          std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
              &attachment,
          const juce::String &paramID) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);

        attachment = std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, paramID, slider);
      };

  setupSlider(pulseFreqSlider, pulseFreqLabel, "Entrainment (Hz)",
              pulseFreqAttachment, "PULSE_FREQ");
  setupSlider(carrierFreqSlider, carrierFreqLabel, "Carrier (Hz)",
              carrierFreqAttachment, "CARRIER_FREQ");
  setupSlider(softnessSlider, softnessLabel, "Softness", softnessAttachment,
              "SOFTNESS");
  setupSlider(gainSlider, gainLabel, "Volume (dB)", gainAttachment, "GAIN");
}

IsochronicToneGenEditor::~IsochronicToneGenEditor() {}

void IsochronicToneGenEditor::paint(juce::Graphics &g) {
  // Gentle gradient to give it a slightly more modern, less "default" look.
  juce::ColourGradient gradient(juce::Colours::darkgrey.darker(0.8f), 0, 0,
                                juce::Colours::black, 0, (float)getHeight(),
                                false);
  g.setGradientFill(gradient);
  g.fillAll();

  g.setColour(juce::Colours::white);
  g.setFont(20.0f);
  g.drawFittedText("Isochronic Tone Generator",
                   getLocalBounds().removeFromTop(40),
                   juce::Justification::centred, 1);

  // Subtle frame around the UI
  g.setColour(juce::Colours::cyan.withAlpha(0.5f));
  g.drawRoundedRectangle(getLocalBounds().reduced(10).toFloat(), 10.0f, 2.0f);
}

void IsochronicToneGenEditor::resized() {
  auto area = getLocalBounds().reduced(20);
  area.removeFromTop(40); // Title space

  auto row1 = area.removeFromTop(area.getHeight() / 2);

  auto leftBlock = row1.removeFromLeft(row1.getWidth() / 2);
  pulseFreqLabel.setBounds(leftBlock.removeFromTop(20));
  pulseFreqSlider.setBounds(leftBlock);

  carrierFreqLabel.setBounds(row1.removeFromTop(20));
  carrierFreqSlider.setBounds(row1);

  auto row2 = area;
  auto leftBlock2 = row2.removeFromLeft(row2.getWidth() / 2);
  softnessLabel.setBounds(leftBlock2.removeFromTop(20));
  softnessSlider.setBounds(leftBlock2);

  gainLabel.setBounds(row2.removeFromTop(20));
  gainSlider.setBounds(row2);
}

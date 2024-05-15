/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CorrelationMeter.h"
#include "VerticalGradientMeter.h"

//==============================================================================
/**
*/
class SimpleCorrelationMeterAudioProcessorEditor  :
    public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    SimpleCorrelationMeterAudioProcessorEditor(
        SimpleCorrelationMeterAudioProcessor&, juce::AudioProcessorValueTreeState& );
    ~SimpleCorrelationMeterAudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleCorrelationMeterAudioProcessor& audioProcessor;
    
    Gui::CorrelationMeter correlationMeter;

    Gui::VerticalGradientMeter verticalGradientMeterL, verticalGradientMeterR;
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::ToggleButton invertLeftButton;
    std::unique_ptr<ButtonAttachment> invertLeftAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleCorrelationMeterAudioProcessorEditor)
};

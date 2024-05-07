/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "HorizontalMeter.h"
#include "CorrelationMeter.h"

//==============================================================================
/**
*/
class SimpleCorrelationMeterAudioProcessorEditor  :
    public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    SimpleCorrelationMeterAudioProcessorEditor (SimpleCorrelationMeterAudioProcessor&);
    ~SimpleCorrelationMeterAudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleCorrelationMeterAudioProcessor& audioProcessor;
    
    Gui::HorizontalMeter horizontalMeterL, horizontalMeterR;
    
    Gui::CorrelationMeter correlationMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleCorrelationMeterAudioProcessorEditor)
};

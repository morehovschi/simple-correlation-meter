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

struct LookAndFeel : juce::LookAndFeel_V4 {
    void drawToggleButton( juce::Graphics& g,
                           juce::ToggleButton& toggleButton,
                           bool shouldDrawToggleButtonAsHighlighted,
                           bool shouldDrawButtonAsDown ) override;
};

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
    std::unique_ptr< ButtonAttachment > invertLeftAttachment;
    juce::ToggleButton invertRightButton;
    std::unique_ptr< ButtonAttachment > invertRightAttachment;
    
    LookAndFeel lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleCorrelationMeterAudioProcessorEditor)
};

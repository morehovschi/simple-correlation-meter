/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleCorrelationMeterAudioProcessorEditor::SimpleCorrelationMeterAudioProcessorEditor (SimpleCorrelationMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible( horizontalMeterL );
    addAndMakeVisible( horizontalMeterR );
     
    setSize (400, 300);
    startTimerHz( 24 );
}

SimpleCorrelationMeterAudioProcessorEditor::~SimpleCorrelationMeterAudioProcessorEditor()
{
}

//==============================================================================
void SimpleCorrelationMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll( juce::Colours::darkgrey );
}

void SimpleCorrelationMeterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    horizontalMeterL.setBounds( 100, 100, 200, 15 );
    horizontalMeterR.setBounds( 100, 120, 200, 15 );
}

void SimpleCorrelationMeterAudioProcessorEditor::timerCallback() {
    horizontalMeterL.setLevel( audioProcessor.getRmsValue( 0 ) );
    horizontalMeterR.setLevel( audioProcessor.getRmsValue( 1 ) );
    
    horizontalMeterL.repaint();
    horizontalMeterR.repaint();
}

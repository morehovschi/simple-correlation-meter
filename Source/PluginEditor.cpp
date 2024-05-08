/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleCorrelationMeterAudioProcessorEditor::SimpleCorrelationMeterAudioProcessorEditor (SimpleCorrelationMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    verticalGradientMeterL( [ & ]() { return audioProcessor.getRmsValue( 0 ); } ),
    verticalGradientMeterR( [ & ]() { return audioProcessor.getRmsValue( 1 ); } )
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible( horizontalMeterL );
    addAndMakeVisible( horizontalMeterR );
    addAndMakeVisible( correlationMeter );
    addAndMakeVisible( verticalGradientMeterL );
    addAndMakeVisible( verticalGradientMeterR );
     
    setSize (400, 500);
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
    correlationMeter.setBounds( 100, 140, 200, 15);
    
    verticalGradientMeterL.setBounds( 100, 200, 15, 200 );
    verticalGradientMeterR.setBounds( 120, 200, 15, 200 );
}

void SimpleCorrelationMeterAudioProcessorEditor::timerCallback() {
    horizontalMeterL.setLevel( audioProcessor.getRmsValue( 0 ) );
    horizontalMeterR.setLevel( audioProcessor.getRmsValue( 1 ) );
    correlationMeter.setCoefficient( audioProcessor.getCorrelationCoefficient() );
    
    horizontalMeterL.repaint();
    horizontalMeterR.repaint();
    correlationMeter.repaint();
}

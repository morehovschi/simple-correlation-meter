/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawToggleButton( juce::Graphics &g,
                                    juce::ToggleButton &toggleButton,
                                    bool shouldDrawToggleButtonAsHighlighted,
                                    bool shouldDrawButtonAsDown ) {
    using namespace juce;
    
    auto color = toggleButton.getToggleState() ? Colours::lightgrey : Colours::dimgrey;
    g.setColour( color );
    auto bounds = toggleButton.getLocalBounds();
    g.drawRect( bounds );
    
    auto text = toggleButton.getButtonText();
    int textHeight = g.getCurrentFont().getHeight();
    int textWidth = g.getCurrentFont().getStringWidth( text );
    g.drawFittedText( text, bounds.getCentreX() - textWidth * 0.5,
        bounds.getCentreY() - textHeight * 0.5, textWidth, textHeight,
        Justification::centred, 1 );
}

//==============================================================================
SimpleCorrelationMeterAudioProcessorEditor::SimpleCorrelationMeterAudioProcessorEditor( SimpleCorrelationMeterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts )
    : AudioProcessorEditor (&p), audioProcessor (p),
    verticalGradientMeterL( [ & ]() { return audioProcessor.getRmsValue( 0 ); },
        true ),
    verticalGradientMeterR( [ & ]() { return audioProcessor.getRmsValue( 1 ); },
        false ),
    valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible( correlationMeter );
    addAndMakeVisible( verticalGradientMeterL );
    addAndMakeVisible( verticalGradientMeterR );
    
    invertLeftButton.setButtonText( "Invert Left" );
    addAndMakeVisible( invertLeftButton );
    invertLeftAttachment.reset( new ButtonAttachment( valueTreeState,
                                                      "Invert Left",
                                                      invertLeftButton ) );
    
    invertRightButton.setButtonText( "Invert Right" );
    addAndMakeVisible( invertRightButton );
    invertRightAttachment.reset( new ButtonAttachment( valueTreeState,
                                                       "Invert Right",
                                                       invertRightButton ) );

    invertLeftButton.setLookAndFeel( &lnf );
    invertRightButton.setLookAndFeel( &lnf );
     
    setSize (400, 600);
    startTimerHz( 24 );
}

SimpleCorrelationMeterAudioProcessorEditor::~SimpleCorrelationMeterAudioProcessorEditor()
{
    invertLeftButton.setLookAndFeel( nullptr );
    invertRightButton.setLookAndFeel( nullptr );
}

void SimpleCorrelationMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll( juce::Colours::darkgrey );
}

void SimpleCorrelationMeterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto correlationMeterArea = bounds.removeFromTop( getHeight() * 0.15f );
    
    correlationMeter.setBounds( correlationMeterArea );
    
    auto leftButtonArea = bounds.removeFromTop( getHeight() * 0.1f );
    auto rightButtonArea = leftButtonArea.removeFromRight(
        bounds.getWidth() * 0.5f );
    
    int horizontalGap = 10;
    int verticalGap = leftButtonArea.getHeight() * 0.1;
    int buttonHeight = leftButtonArea.getHeight() * 0.5;
    int buttonWidth = buttonHeight * 3;
    
    invertLeftButton.setBounds(
        leftButtonArea.getX() + leftButtonArea.getWidth() - horizontalGap -
            buttonWidth,
        leftButtonArea.getY() + verticalGap,
        buttonWidth,
        buttonHeight );
    
    invertRightButton.setBounds(
        rightButtonArea.getX() + horizontalGap,
        rightButtonArea.getY() + verticalGap,
        buttonWidth,
        buttonHeight );
    
    auto leftMeterArea = bounds.removeFromTop( bounds.getHeight() * 0.6 );
    auto rightMeterArea = leftMeterArea.removeFromRight(
        getWidth() * 0.5 );
        
    verticalGradientMeterL.setBounds( leftMeterArea );
    verticalGradientMeterR.setBounds( rightMeterArea );
}

void SimpleCorrelationMeterAudioProcessorEditor::timerCallback() {

    correlationMeter.setCoefficient( audioProcessor.getCorrelationCoefficient() );
    correlationMeter.setMinimumCorrelation(
        audioProcessor.getMinimumCorrelation() );
    
    correlationMeter.repaint();
}
//==============================================================================

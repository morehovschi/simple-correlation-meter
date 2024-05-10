/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleCorrelationMeterAudioProcessor::SimpleCorrelationMeterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleCorrelationMeterAudioProcessor::~SimpleCorrelationMeterAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleCorrelationMeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleCorrelationMeterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleCorrelationMeterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleCorrelationMeterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleCorrelationMeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleCorrelationMeterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleCorrelationMeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleCorrelationMeterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleCorrelationMeterAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleCorrelationMeterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleCorrelationMeterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    rmsLevelLeft.reset( sampleRate, 0.5f );
    rmsLevelRight.reset( sampleRate, 0.5f );
    correlation.reset( sampleRate, 0.15f );
    
    rmsLevelLeft.setCurrentAndTargetValue( -100.f );
    rmsLevelRight.setCurrentAndTargetValue( -100.f );
    correlation.setCurrentAndTargetValue( 0.f );
}

void SimpleCorrelationMeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleCorrelationMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

static float computeCorrelation( const float* x, const float* y, int numSamples ) {
    // get mean for left and right channels
    float meanX = 0.f, meanY = 0.f;
    for ( int i = 0; i < numSamples; i++ ){
        meanX += x[ i ];
        meanY = y[ i ];
    }
    meanX /= numSamples;
    meanY /= numSamples;
    
    // calculate Pearson correlation coefficient
    float numerator = 0.f, leftSumSquared = 0.f, rightSumSquared = 0.f;
    float leftDiff = 0.f, rightDiff = 0.f;
    for ( int i = 0; i < numSamples; i++ ){
        leftDiff = x[ i ] - meanX;
        rightDiff = y[ i ] - meanY;
        
        numerator += leftDiff * rightDiff;
        leftSumSquared += leftDiff * leftDiff;
        rightSumSquared += rightDiff * rightDiff;
    }
    float correlation = numerator / std::sqrt( leftSumSquared * rightSumSquared );
    
    // if correlation within range
    if ( correlation >= -1.f && correlation <= 1.f ) {
        return correlation;
    }
    
    // NaN guard
    return 0.f;
}

void SimpleCorrelationMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    using namespace juce;

    rmsLevelLeft.skip( buffer.getNumSamples() );
    rmsLevelRight.skip( buffer.getNumSamples() );
    correlation.skip( buffer.getNumSamples() );
    
    {
        const auto value = Decibels::gainToDecibels(
            buffer.getRMSLevel( 0, 0, buffer.getNumSamples() ) );
        if ( value < rmsLevelLeft.getCurrentValue() )
            rmsLevelLeft.setTargetValue( value );
        else
            rmsLevelLeft.setCurrentAndTargetValue( value );
    }

    {
        const auto value = Decibels::gainToDecibels(
            buffer.getRMSLevel( 1, 0, buffer.getNumSamples() ) );
        if ( value < rmsLevelRight.getCurrentValue() )
            rmsLevelRight.setTargetValue( value );
        else
            rmsLevelRight.setCurrentAndTargetValue( value );
    }
    
    // calculate correlation
    correlation.setTargetValue( computeCorrelation( buffer.getReadPointer( 0 ),
                                    buffer.getReadPointer( 1 ),
                                    buffer.getNumSamples() ) );
    
    float currentCorrelation = correlation.getCurrentValue();
    if ( currentCorrelation < 0 ) {
        if ( ( minimumCorrelation == -2.f ) ||
             ( currentCorrelation < minimumCorrelation ) ) {
            minimumCorrelation = currentCorrelation;
        }
    }
    
    if ( ( buffer.getMagnitude( 0, 0, buffer.getNumSamples() ) ) == 0 &&
         ( buffer.getMagnitude( 1, 0, buffer.getNumSamples() ) ) == 0 ) {
         silentBufferCount += 1;
    } else {
        silentBufferCount = 0;
    }
    
    // if 1 second of complete silence
    if ( silentBufferCount >= getSampleRate() / buffer.getNumSamples() ) {
        // set to lower value to prevent overflow
        silentBufferCount = 1;
        // reset to sentinel value
        minimumCorrelation = -2.f;
    }

}

//==============================================================================
bool SimpleCorrelationMeterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleCorrelationMeterAudioProcessor::createEditor()
{
    return new SimpleCorrelationMeterAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleCorrelationMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleCorrelationMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


float SimpleCorrelationMeterAudioProcessor::getRmsValue( const int channel ) const {
    jassert( channel == 0 || channel == 1 );
    
    if ( channel == 0 )
        return rmsLevelLeft.getCurrentValue();
    if ( channel == 1 )
        return rmsLevelRight.getCurrentValue();
    
    return 0.f;
}

float SimpleCorrelationMeterAudioProcessor::getCorrelationCoefficient() const {
    return correlation.getCurrentValue();
}

float SimpleCorrelationMeterAudioProcessor::getMinimumCorrelation() const { 
    return minimumCorrelation;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleCorrelationMeterAudioProcessor();
}

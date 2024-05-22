/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const float CORRELATION_RAMP = 0.15f;

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
                       ),
#endif
parameters( *this, nullptr, juce::Identifier( "SimpleCorrelationMeter" ),
            { std::make_unique< juce::AudioParameterBool >(
                juce::ParameterID{ "Invert Left", 1 }, "Invert Left", false ),
              std::make_unique< juce::AudioParameterBool >(
                juce::ParameterID{ "Invert Right", 1 }, "Invert Right", false ) } )
{
    invertLeft = parameters.getRawParameterValue( "Invert Left" );
    invertRight = parameters.getRawParameterValue( "Invert Right" );
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
    
    // set up the linear smoothed values with the interval to smooth over
    rmsLevelLeft.reset( sampleRate, 0.5f );
    rmsLevelRight.reset( sampleRate, 0.5f );
    correlationIn.reset( sampleRate, CORRELATION_RAMP );
    correlationOut.reset( sampleRate, CORRELATION_RAMP );
    
    rmsLevelLeft.setCurrentAndTargetValue( -100.f );
    rmsLevelRight.setCurrentAndTargetValue( -100.f );
    correlationIn.setCurrentAndTargetValue( 0.f );
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

/*
helper that unwraps Optional values;
from: https://forum.juce.com/t/juce-optional-get/51982/2
*/
template <typename Value>
Value get( juce::Optional<Value> opt, Value fallback = {} )
{
    if ( opt.hasValue() )
        return *opt;

    jassertfalse;
    return fallback;
}

void SimpleCorrelationMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    using namespace juce;

    int bufferSize = buffer.getNumSamples();
    rmsLevelLeft.skip( bufferSize );
    rmsLevelRight.skip( bufferSize );
    correlationIn.skip( bufferSize );
    correlationOut.skip( bufferSize );
    
    {
        const auto value = Decibels::gainToDecibels(
            buffer.getRMSLevel( 0, 0, bufferSize ) );
        if ( value < rmsLevelLeft.getCurrentValue() )
            rmsLevelLeft.setTargetValue( value );
        else
            rmsLevelLeft.setCurrentAndTargetValue( value );
    }

    {
        const auto value = Decibels::gainToDecibels(
            buffer.getRMSLevel( 1, 0, bufferSize ) );
        if ( value < rmsLevelRight.getCurrentValue() )
            rmsLevelRight.setTargetValue( value );
        else
            rmsLevelRight.setCurrentAndTargetValue( value );
    }
    
    // calculate correlation
    correlationIn.setTargetValue( computeCorrelation( buffer.getReadPointer( 0 ),
                                    buffer.getReadPointer( 1 ),
                                    bufferSize ) );
    
    float currentCorrelationIn = correlationIn.getCurrentValue();
    if ( currentCorrelationIn < 0 ) {
        if ( ( minCorrelationIn == -2.f ) ||
             ( currentCorrelationIn < minCorrelationIn ) ) {
            minCorrelationIn = currentCorrelationIn;
        }
    }
    
    // invert left channel phase
    if ( *invertLeft > 0.5f ) {
        if ( previouslyInvertedLeft ) {
            auto* leftBuffer = buffer.getWritePointer( 0 );
            
            for ( int i = 0; i < bufferSize; i++ ) { // consider storing numSamples variable
                leftBuffer[ i ] *= -1.f;
            }
        } else {
            // to avoid creating clicks
            buffer.applyGainRamp( 0, 0, bufferSize, 1.f, -1.f );
            // reset displayed current minimum correlation
            
            minCorrelationOut = -2.f;
            outCorrelationWait = CORRELATION_RAMP * getSampleRate();
        }
    } else {
        if ( previouslyInvertedLeft ) {
            buffer.applyGainRamp( 0, 0, bufferSize, -1.f, 1.f );
            
            minCorrelationOut = -2.f;
            outCorrelationWait = CORRELATION_RAMP * getSampleRate();
        }
    }
    previouslyInvertedLeft = *invertLeft;
    
    // invert right channel phase
    if ( *invertRight > 0.5f ) {
        if ( previouslyInvertedRight ) {
            auto* rightBuffer = buffer.getWritePointer( 1 );
            
            for ( int i = 0; i < bufferSize; i++ ) { // consider storing numSamples variable
                rightBuffer[ i ] *= -1.f;
            }
        } else {
            buffer.applyGainRamp( 1, 0, bufferSize, 1.f, -1.f );
            
            minCorrelationOut = -2.f;
            outCorrelationWait = CORRELATION_RAMP * getSampleRate();
        }
    } else {
        if ( previouslyInvertedRight ) {
            buffer.applyGainRamp( 1, 0, bufferSize, -1.f, 1.f );
            
            minCorrelationOut = -2.f;
            outCorrelationWait = CORRELATION_RAMP * getSampleRate();
        }
    }
    previouslyInvertedRight = *invertRight;
    
    // no need to calculate correlation-out sample-by-sample – it is the negative of
    // correlation-in if left or right has been inverted, otherwise it's the same
    if ( *invertLeft != *invertRight ) {
        correlationOut.setTargetValue( correlationIn.getTargetValue() * ( -1.f ) );
    } else {
        correlationOut.setTargetValue( correlationIn.getTargetValue() );
    }
    
    float currentCorrelationOut = correlationOut.getCurrentValue();
    if ( currentCorrelationOut < 0 ) {
        if ( ( outCorrelationWait == 0 ) && // when this value non-zero => ramping
             ( ( ( minCorrelationOut == -2.f ) || // -2.0 => sentinel value
               ( currentCorrelationOut < minCorrelationOut ) ) ) ) {
            minCorrelationOut = currentCorrelationOut;
        }
    }
    
    auto playhead = getPlayHead();
	if ( playhead != nullptr ) // playhead may not always exist
	{
        if ( playhead->getPosition() ) {
            auto info = get( playhead->getPosition() ); // unwrap Optional value
            
            // reset displayed min correlation when transitioning to playing
            if ( info.getIsPlaying() && ( previouslyPlaying == false ) ) {
                // reset to sentinel value
                minCorrelationIn = -2.f;
                minCorrelationOut = -2.f;
            }
            
            previouslyPlaying = info.getIsPlaying();
        }
	}
    
    // if ramping to or from inverted value, decrement the wait counter
    if ( outCorrelationWait ) {
        outCorrelationWait = jmax( outCorrelationWait - bufferSize, 0 );
    }
}

//==============================================================================
bool SimpleCorrelationMeterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleCorrelationMeterAudioProcessor::createEditor()
{
    return new SimpleCorrelationMeterAudioProcessorEditor (*this, parameters );
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

float SimpleCorrelationMeterAudioProcessor::getCorrelationIn() const {
    return correlationIn.getCurrentValue();
}

float SimpleCorrelationMeterAudioProcessor::getCorrelationOut() const {
    return correlationOut.getCurrentValue();
}

float SimpleCorrelationMeterAudioProcessor::getMinCorrelationIn() const { 
    return minCorrelationIn;
}

float SimpleCorrelationMeterAudioProcessor::getMinCorrelationOut() const {
    return minCorrelationOut;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleCorrelationMeterAudioProcessor();
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SimpleCorrelationMeterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleCorrelationMeterAudioProcessor();
    ~SimpleCorrelationMeterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float getRmsValue( const int channel ) const;
    
    float getCorrelationIn() const;
    float getCorrelationOut() const;
    
    float getMinCorrelationIn() const;
    float getMinCorrelationOut() const;
    
private:
    juce::LinearSmoothedValue< float >
        rmsLevelLeft, rmsLevelRight, correlationIn, correlationOut;
    
    // initialized to sentinel value
    float minCorrelationIn = -2.f;
    float minCorrelationOut = -2.f;
    
    // to detect transitions from paused audio to playing
    bool previouslyPlaying = false;
    
    // to not show correlation-out right away, but wait for ramp
    int outCorrelationWait = 0;
    
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic< float >* invertLeft = nullptr;
    std::atomic< float >* invertRight = nullptr;
    
    bool previouslyInvertedLeft = false;
    bool previouslyInvertedRight = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleCorrelationMeterAudioProcessor)
};

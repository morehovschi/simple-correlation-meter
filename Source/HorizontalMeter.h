/*
  ==============================================================================

    HorizontalMeter.h
    Created: 5 May 2024 3:05:29pm
    Author:  Marius Orehovschi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui {
    class HorizontalMeter : public juce::Component {
    
    public:
        void paint( juce::Graphics& g ) override {
            using namespace juce;
            
            auto bounds = getLocalBounds().toFloat();
            
            g.setColour( Colours::white.withBrightness( 0.4f ) );
            g.fillRoundedRectangle( bounds, 5.f );
            
            g.setColour( Colours::white );
            const auto scaledX = jmap( level, -60.f, 6.f, 0.f,
                static_cast< float >( getWidth() ) );
            g.fillRoundedRectangle( bounds.removeFromLeft( scaledX ), 5.f );
        }
        
        void setLevel( const float value ) { level = value; }
    private:
        float level = -60.f;
    };
}

/*
  ==============================================================================

    CorrelationMeter.h
    Created: 7 May 2024 11:00:18am
    Author:  Marius Orehovschi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace Gui {
    class CorrelationMeter : public juce::Component {
    
    public:
        void paint( juce::Graphics& g ) override {
            using namespace juce;
            
            auto bounds = getLocalBounds().toFloat();
            
            g.setColour( Colours::white.withBrightness( 0.4f ) );
            g.fillRoundedRectangle( bounds, 5.f );
            
            g.setColour( Colours::white );
            
            float usedCoefficient = 0.f;
            
            // if computed coefficient within range (guard against NaN)
            if ( ( coefficient >= -1.f && coefficient <= 1.f ) ) {
                usedCoefficient = coefficient;
            }
            
            const auto scaledX = jmap( usedCoefficient, -1.f, 1.f, 0.f,
                static_cast< float >( getWidth() ) );
            
            g.fillRoundedRectangle( bounds.getX() + scaledX - 1,
                                    bounds.getY(),
                                    2,
                                    bounds.getHeight(),
                                    5.f);
        }
        
        void setCoefficient( const float value ) { coefficient = value; }
    private:
        float coefficient = 0.f;
    };
}

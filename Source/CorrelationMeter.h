/*
  ==============================================================================

    CorrelationMeter.h
    Created: 7 May 2024 11:00:18am
    Author:  Marius Orehovschi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui {
    class CorrelationMeter : public juce::Component {
    
    public:
        void paint( juce::Graphics& g ) override {
            using namespace juce;
            
            auto bounds = getLocalBounds().toFloat();
            auto meterDisplay = bounds.reduced(
                bounds.getWidth() * 0.1, bounds.getHeight() * 0.42 );
            
            g.setColour( Colours::white.withBrightness( 0.4f ) );
            g.fillRoundedRectangle( meterDisplay, 3.f );
            
            // mark 0.0 correlation point
            g.setColour( Colours::white.withBrightness( 0.7f ) );
            g.fillRoundedRectangle(
                meterDisplay.getX() + ( meterDisplay.getWidth() * 0.5 - 1 ),
                meterDisplay.getY(),
                2,
                meterDisplay.getHeight(),
                0.5f );
            
            // mark -0.5 and 0.5 points
            g.setColour( Colours::white.withBrightness( 0.5f ) );
            g.fillRoundedRectangle(
                meterDisplay.getX() + ( meterDisplay.getWidth() * 0.25 - 1 ),
                meterDisplay.getY(),
                2,
                meterDisplay.getHeight(),
                0.5f );
            g.setColour( Colours::white.withBrightness( 0.5f ) );
            g.fillRoundedRectangle(
                meterDisplay.getX() + ( meterDisplay.getWidth() * 0.75 - 1 ),
                meterDisplay.getY(),
                2,
                meterDisplay.getHeight(),
                0.5f );
                
            std::vector< float > positions{ -1.f, -0.5f, 0.f, 0.5f, 1.f };
            for( auto pos : positions ) {
                juce::String str( pos, 1, false );
                
                int textHeight = g.getCurrentFont().getHeight();
                int textWidth = g.getCurrentFont().getStringWidth( str );
                
                juce::Rectangle< int > r;
                r.setSize( textWidth, textHeight );
                
                r.setX( meterDisplay.getX() +
                    meterDisplay.getWidth() * ( 0.5 + 0.5 * pos ) - textWidth * 0.5 );
                r.setY( meterDisplay.getY() - textHeight - 2 );
                
                g.drawFittedText( str, r, juce::Justification::centred, 1 );
            }
            
            
            
            // draw the needle of the meter
            g.setColour( Colours::white );
            const auto scaledX = jmap( coefficient, -1.f, 1.f, 0.f,
                static_cast< float >( meterDisplay.getWidth() ) );
            g.fillRoundedRectangle( meterDisplay.getX() + scaledX - 1,
                                    meterDisplay.getY(),
                                    2,
                                    meterDisplay.getHeight(),
                                    5.f );
        }
        
        void setCoefficient( const float value ) { coefficient = value; }
    private:
        float coefficient = 0.f;
    };
}

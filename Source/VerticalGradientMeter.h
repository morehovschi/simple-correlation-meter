/*
  ==============================================================================

    VerticalGradientMeter.h
    Created: 7 May 2024 3:38:58pm
    Author:  Marius Orehovschi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui {
    class VerticalGradientMeter :
        public juce::Component,
        public juce::Timer {
    public:
        VerticalGradientMeter( std::function< float() >&& valueFunction, bool leftMeter ) :
                valueSupplier( std::move( valueFunction ) ),
                leftMeter( leftMeter ) {
            startTimerHz( 24 );
        }
        void paint( juce::Graphics& g ) override {
            using namespace juce;
        
            const auto level = valueSupplier();
            
            auto bounds = getDisplayBounds();
            
            std::vector< float > positions{ 0.f, -15.f, -30.f, -45.f };
            for ( auto position: positions ) {
                g.setColour( Colours::lightgrey.withBrightness( 0.5f ) );
                
                const auto height =
                    jmap( position, 6.f, -60.f, 0.f,
                        static_cast< float >( bounds.getHeight()  ) );
                
                String label( position, 1, false );
                label << " dB";
                
                int labelX = leftMeter?
                    bounds.getX() - g.getCurrentFont().getStringWidth( label ) - 10 :
                    bounds.getX() + bounds.getWidth() + 10;
                
                g.drawFittedText( label,
                                  labelX,
                                  height,
                                  g.getCurrentFont().getStringWidth( label ),
                                  g.getCurrentFont().getHeight(),
                                  Justification::centred,
                                  1 );
                
                if ( position == 0.f ) {
                    g.setColour( Colours::lightgrey );
                }
                g.drawHorizontalLine( height,
                                      ( float ) bounds.getX(),
                                      ( float ) bounds.getX() + bounds.getWidth() );
            }
            
            g.setGradientFill( gradient );
            const auto scaledY =
                jmap( level, -60.f, 6.f, 0.f,
                      static_cast< float >( bounds.getHeight() ) );
            if ( level >= -59.9f )
                // avoids drawing a green line at the bottom when no signal
                g.fillRect( bounds.removeFromBottom( scaledY ) );
            
            bounds = getDisplayBounds();
            
            // draw meter boundaries
            g.setColour( Colours::lightgrey.withBrightness( 0.5f ) );
            g.drawRect( bounds );
        }
        
        void resized() override {
            using namespace juce;
            
            const auto bounds = getDisplayBounds();
            
            gradient = ColourGradient {
                Colours::green,
                bounds.getBottomLeft(),
                Colours::red,
                bounds.getTopLeft(),
                false
            };
            
            gradient.addColour( 0.5, Colours::yellow );
        }
        
        juce::Rectangle< float > getDisplayBounds() const {
            auto bounds = getLocalBounds().toFloat();
            
            int width = bounds.getWidth();
            
            float meterX = bounds.getX() + width * ( leftMeter? 0.8f : 0.05f );
            float meterWidth = width * 0.15f;
            
            return juce::Rectangle( meterX,
                                    bounds.getY(),
                                    meterWidth,
                                    bounds.getHeight() - 1 );
        }
        
        void timerCallback() override {
            repaint();
        }
    
    private:
        std::function< float() > valueSupplier;
        bool leftMeter;
    
        juce::ColourGradient gradient{};
    };
}

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
        VerticalGradientMeter( std::function< float() >&& valueFunction ) :
                valueSupplier( std::move( valueFunction ) ) {
            startTimerHz( 24 );
        }
        void paint( juce::Graphics& g ) override {
            using namespace juce;
        
            const auto level = valueSupplier();
            
            auto bounds = getLocalBounds().toFloat();
            
            g.setColour( Colours::black );
            g.fillRect( bounds );
            
            g.setGradientFill( gradient );
            const auto scaledY =
                jmap( level, -60.f, 6.f, 0.f, static_cast< float >( getHeight() ) );
            
            g.fillRect( bounds.removeFromBottom( scaledY ) );
        }
        
        void resized() override {
            using namespace juce;
            
            const auto bounds = getLocalBounds().toFloat();
            
            gradient = ColourGradient {
                Colours::green,
                bounds.getBottomLeft(),
                Colours::red,
                bounds.getTopLeft(),
                false
            };
            
            gradient.addColour( 0.5, Colours::yellow );
        }
        
        void timerCallback() override {
            repaint();
        }
    
    private:
        std::function< float() > valueSupplier;
    
        juce::ColourGradient gradient{};
    };
}

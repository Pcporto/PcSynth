/*
  ==============================================================================

    Editor UI - Sintetizador Aditivo Espectral Modal

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    Visualizador de espectro em tempo real
*/
class EspectroVisualizer : public juce::Component, public juce::Timer
{
public:
    EspectroVisualizer (PluginProcessor& processor);
    ~EspectroVisualizer() override;

    void paint (juce::Graphics& g) override;
    void timerCallback() override;
    void resized() override;

private:
    PluginProcessor& audioProcessor;
    std::vector<float> espectroBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EspectroVisualizer)
};

//==============================================================================
/**
    Editor principal do plugin com interface dark mode roxo
*/
class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::Slider::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* slider) override;

private:
    //==============================================================================
    PluginProcessor& audioProcessor;

    // Sliders
    juce::Slider sliderAmplitude;
    juce::Slider sliderModalDecay;
    juce::Slider sliderBrightness;

    // Labels
    juce::Label labelAmplitude;
    juce::Label labelModalDecay;
    juce::Label labelBrightness;
    juce::Label labelTitle;

    // Visualizador
    std::unique_ptr<EspectroVisualizer> espectroVisualizer;

    // Attachment para APVTS
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attachmentAmplitude;
    std::unique_ptr<SliderAttachment> attachmentModalDecay;
    std::unique_ptr<SliderAttachment> attachmentBrightness;

    //==============================================================================
    void setupSlider (juce::Slider& slider, const juce::String& label,
                      float minVal, float maxVal, float defaultVal);
    void setupLabel (juce::Label& label, const juce::String& text);
    void applyDarkTheme();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

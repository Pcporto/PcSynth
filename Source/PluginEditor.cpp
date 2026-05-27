/*
  ==============================================================================

    Implementação do Editor UI

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// ESPECTRO VISUALIZER
//==============================================================================

EspectroVisualizer::EspectroVisualizer (PluginProcessor& processor)
    : audioProcessor (processor)
{
    espectroBuffer.resize (256, 0.0f);
    startTimer (30); // ~33 FPS
}

EspectroVisualizer::~EspectroVisualizer()
{
    stopTimer();
}

void EspectroVisualizer::paint (juce::Graphics& g)
{
    // Fundo dark
    g.fillAll (juce::Colour::fromRGB (15, 15, 25));

    // Desenhar grade
    g.setColour (juce::Colour::fromRGB (50, 30, 70).withAlpha (0.3f));
    int numLinhas = 8;
    for (int i = 0; i <= numLinhas; ++i)
    {
        float y = (getHeight() / numLinhas) * i;
        g.drawHorizontalLine ((int)y, 0.0f, (float)getWidth());
    }

    // Desenhar espectro
    g.setColour (juce::Colour::fromRGB (200, 100, 255)); // Roxo vibrante
    
    float barWidth = getWidth() / (float)espectroBuffer.size();
    
    for (size_t i = 0; i < espectroBuffer.size(); ++i)
    {
        float valor = espectroBuffer[i];
        float x = barWidth * i;
        float altura = valor * getHeight();
        
        g.fillRect (x, getHeight() - altura, barWidth - 1.0f, altura);
    }

    // Borda roxo
    g.setColour (juce::Colour::fromRGB (150, 50, 200).withAlpha (0.5f));
    g.drawRect (getLocalBounds(), 2);
}

void EspectroVisualizer::timerCallback()
{
    // Simular dados de espectro
    for (size_t i = 0; i < espectroBuffer.size(); ++i)
    {
        float random = juce::Random::getSystemRandom().nextFloat();
        espectroBuffer[i] = espectroBuffer[i] * 0.7f + random * 0.3f;
    }
    
    repaint();
}

void EspectroVisualizer::resized()
{
}

//==============================================================================
// PLUGIN EDITOR
//==============================================================================

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (500, 700);
    
    // Aplicar tema dark
    applyDarkTheme();

    // Título
    addAndMakeVisible (labelTitle);
    setupLabel (labelTitle, "PcSynth - Síntese Modal Aditiva");
    labelTitle.setFont (juce::Font (24.0f, juce::Font::bold));
    labelTitle.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 100, 255));

    // Setup Sliders
    addAndMakeVisible (sliderAmplitude);
    setupSlider (sliderAmplitude, "Master Amplitude", 0.0f, 1.0f, 0.5f);
    addAndMakeVisible (labelAmplitude);
    setupLabel (labelAmplitude, "Amplitude");

    addAndMakeVisible (sliderModalDecay);
    setupSlider (sliderModalDecay, "Modal Decay", 0.01f, 0.9999f, 0.95f);
    addAndMakeVisible (labelModalDecay);
    setupLabel (labelModalDecay, "Decay (Q)");

    addAndMakeVisible (sliderBrightness);
    setupSlider (sliderBrightness, "Spectral Brightness", 0.0f, 1.0f, 1.0f);
    addAndMakeVisible (labelBrightness);
    setupLabel (labelBrightness, "Brightness");

    // Visualizador
    espectroVisualizer = std::make_unique<EspectroVisualizer> (audioProcessor);
    addAndMakeVisible (*espectroVisualizer);

    // Attachments APVTS
    attachmentAmplitude = std::make_unique<SliderAttachment> 
        (audioProcessor.apvts, "masterAmplitude", sliderAmplitude);
    attachmentModalDecay = std::make_unique<SliderAttachment>
        (audioProcessor.apvts, "modalDecay", sliderModalDecay);
    attachmentBrightness = std::make_unique<SliderAttachment>
        (audioProcessor.apvts, "spectralBrightness", sliderBrightness);
}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    // Background dark com gradiente
    g.fillAll (juce::Colour::fromRGB (12, 12, 20));
    
    // Efeito de gradiente sutil
    juce::ColourGradient gradient (juce::Colour::fromRGB (20, 10, 30),
                                    0.0f, 0.0f,
                                    juce::Colour::fromRGB (12, 12, 20),
                                    0.0f, (float)getHeight(),
                                    false);
    g.setGradientFill (gradient);
    g.fillAll();
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (20);

    // Título
    labelTitle.setBounds (area.removeFromTop (50));

    // Espaço
    area.removeFromTop (20);

    // Visualizador de espectro
    espectroVisualizer->setBounds (area.removeFromTop (120));
    area.removeFromTop (20);

    // Sliders verticais
    auto sliderHeight = 80;
    
    // Amplitude
    auto amplitudeArea = area.removeFromTop (sliderHeight + 30);
    labelAmplitude.setBounds (amplitudeArea.removeFromTop (25));
    sliderAmplitude.setBounds (amplitudeArea);
    area.removeFromTop (10);

    // Modal Decay
    auto decayArea = area.removeFromTop (sliderHeight + 30);
    labelModalDecay.setBounds (decayArea.removeFromTop (25));
    sliderModalDecay.setBounds (decayArea);
    area.removeFromTop (10);

    // Brightness
    auto brightnessArea = area.removeFromTop (sliderHeight + 30);
    labelBrightness.setBounds (brightnessArea.removeFromTop (25));
    sliderBrightness.setBounds (brightnessArea);
}

void PluginEditor::sliderValueChanged (juce::Slider* slider)
{
    // Já conectado via APVTS, não precisa fazer nada aqui
}

//==============================================================================
void PluginEditor::setupSlider (juce::Slider& slider, const juce::String& label,
                                 float minVal, float maxVal, float defaultVal)
{
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    slider.setRange (minVal, maxVal, 0.01f);
    slider.setValue (defaultVal);
    slider.addListener (this);

    // Styling roxo
    slider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB (200, 100, 255));
    slider.setColour (juce::Slider::trackColourId, juce::Colour::fromRGB (100, 50, 150));
    slider.setColour (juce::Slider::backgroundColourId, juce::Colour::fromRGB (30, 20, 50));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB (100, 50, 150));
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour::fromRGB (200, 100, 255));
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB (20, 10, 35));
}

void PluginEditor::setupLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 100, 255));
    label.setFont (juce::Font (14.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
}

void PluginEditor::applyDarkTheme()
{
    // Define o Look and Feel global
    getLookAndFeel().setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB (200, 100, 255));
    getLookAndFeel().setColour (juce::Slider::trackColourId, juce::Colour::fromRGB (100, 50, 150));
    getLookAndFeel().setColour (juce::Slider::backgroundColourId, juce::Colour::fromRGB (30, 20, 50));
}

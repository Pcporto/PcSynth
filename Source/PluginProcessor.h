/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModalOscillator.h"

//==============================================================================
/**
    Processador principal do sintetizador aditivo espectral com síntese modal
*/
class PluginProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PluginProcessor();
    ~PluginProcessor() override;

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

    //==============================================================================
    // Acesso aos parâmetros APVTS
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    // Parâmetros de síntese modal aditiva
    static constexpr int NUM_PARCIAIS = 32;
    
    std::array<ModalOscillator, NUM_PARCIAIS> osciladores;
    
    double taxaAmostragem = 44100.0;
    int amostrasPerBlock = 512;
    
    // Parâmetros de síntese
    float amplitudeMestre = 0.5f;
    float decaimentoModal = 0.95f;      // Q factor para amortecimento
    float brilhoEspectral = 1.0f;       // Controla brilho (corte de altas frequências)
    
    //==============================================================================
    // Criar árvore de parâmetros APVTS
    juce::AudioProcessorValueTreeState::ParameterLayout criarLayout();
    
    // Processar nota MIDI
    void processarNotaMidi (int nota, int velocidade, bool ativo);
    
    // Atualizar parâmetros de síntese
    void atualizarParametros();
    
    // Sintetizar bloco de áudio com osciladores modais
    void sintetizarBloco (juce::AudioBuffer<float>& buffer, int numAmostras);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

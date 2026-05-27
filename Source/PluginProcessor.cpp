/*
  ==============================================================================

    Processador de Áudio - Sintetizador Aditivo Espectral Modal

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "PARAMETERS", criarLayout())
#endif
{
    // Inicializar osciladores
    for (int i = 0; i < NUM_PARCIAIS; ++i)
    {
        osciladores[i].setIndiceParcial (i + 1);
    }
}

//==============================================================================
PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    // Tempo de "cauda" do áudio (decay modal)
    return 5.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginProcessor::getProgramName (int index)
{
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    taxaAmostragem = sampleRate;
    amostrasPerBlock = samplesPerBlock;
    
    // Preparar todos os osciladores
    for (int i = 0; i < NUM_PARCIAIS; ++i)
    {
        osciladores[i].inicializar (69, static_cast<float> (sampleRate)); // Lá 4 (A4)
    }
}

//==============================================================================
void PluginProcessor::releaseResources()
{
}

//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Atualizar parâmetros
    atualizarParametros();
    
    // Limpar buffer de saída
    for (int canal = 0; canal < totalNumOutputChannels; ++canal)
        buffer.clear (canal, 0, buffer.getNumSamples());
    
    // Processar mensagens MIDI
    for (auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        
        if (msg.isNoteOn())
        {
            processarNotaMidi (msg.getNoteNumber(), msg.getVelocity(), true);
        }
        else if (msg.isNoteOff())
        {
            processarNotaMidi (msg.getNoteNumber(), msg.getVelocity(), false);
        }
    }
    
    // Sintetizar bloco
    sintetizarBloco (buffer, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::criarLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Amplitude Mestre
    params.push_back (std::make_unique<juce::AudioParameterFloat> 
        ("masterAmplitude", "Master Amplitude", 0.0f, 1.0f, 0.5f));
    
    // Decaimento Modal (Q Factor)
    params.push_back (std::make_unique<juce::AudioParameterFloat> 
        ("modalDecay", "Modal Decay", 0.01f, 0.9999f, 0.95f));
    
    // Brilho Espectral
    params.push_back (std::make_unique<juce::AudioParameterFloat> 
        ("spectralBrightness", "Spectral Brightness", 0.0f, 1.0f, 1.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
void PluginProcessor::processarNotaMidi (int nota, int velocidade, bool ativo)
{
    // Mapear velocidade MIDI para amplitude (0-127 -> 0-1)
    float amplidadeNota = velocidade / 127.0f;
    
    // Distribuir a energia entre os parciais com uma envolvente espectral
    for (int i = 0; i < NUM_PARCIAIS; ++i)
    {
        osciladores[i].inicializar (nota, static_cast<float> (taxaAmostragem));
        
        if (ativo)
        {
            // Amplitude decresce com os harmônicos (1/n)
            float amplidadeParcial = amplidadeNota / (i + 1.0f);
            osciladores[i].setAmplitude (amplidadeParcial);
        }
        else
        {
            osciladores[i].setAmplitude (0.0f);
        }
    }
}

//==============================================================================
void PluginProcessor::atualizarParametros()
{
    amplitudeMestre = apvts.getRawParameterValue ("masterAmplitude")->load();
    decaimentoModal = apvts.getRawParameterValue ("modalDecay")->load();
    brilhoEspectral = apvts.getRawParameterValue ("spectralBrightness")->load();
    
    // Aplicar parâmetros a todos os osciladores
    for (int i = 0; i < NUM_PARCIAIS; ++i)
    {
        osciladores[i].setQFactor (decaimentoModal);
        osciladores[i].setBrilhoEspectral (brilhoEspectral);
    }
}

//==============================================================================
void PluginProcessor::sintetizarBloco (juce::AudioBuffer<float>& buffer, int numAmostras)
{
    auto numCanais = buffer.getNumChannels();
    
    // Processar cada oscilador em cada canal
    for (int i = 0; i < NUM_PARCIAIS; ++i)
    {
        for (int canal = 0; canal < numCanais; ++canal)
        {
            auto* dados = buffer.getWritePointer (canal);
            
            for (int amostra = 0; amostra < numAmostras; ++amostra)
            {
                dados[amostra] += osciladores[i].processar() * amplitudeMestre;
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

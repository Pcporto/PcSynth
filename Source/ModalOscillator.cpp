/*
  ==============================================================================

    Implementação do Oscilador Modal

  ==============================================================================
*/

#include "ModalOscillator.h"

//==============================================================================
void ModalOscillator::inicializar (int notaMidi, float sampleRate)
{
    taxaAmostragem = sampleRate;
    
    // Converter nota MIDI para frequência em Hz
    // Fórmula: f = 440 * 2^((nota - 69) / 12)
    frequenciaHz = 440.0f * std::pow (2.0f, (notaMidi - 69.0f) / 12.0f);
    
    // Calcular incremento de fase: 2π * f / fs
    incrementoFase = 2.0f * juce::MathConstants<float>::pi * frequenciaHz / taxaAmostragem;
    
    fase = 0.0f;
    envolvente = 1.0f;
    amplitude = 0.0f;
}

//==============================================================================
void ModalOscillator::setAmplitude (float amp)
{
    amplitudeAlvo = juce::jlimit (0.0f, 1.0f, amp);
}

//==============================================================================
void ModalOscillator::setQFactor (float q)
{
    qFactor = juce::jlimit (0.01f, 0.9999f, q);
    // Taxa de decaimento baseada no Q factor
    // Quanto maior o Q, mais lento o decaimento
    taxaDecaimento = qFactor;
}

//==============================================================================
void ModalOscillator::setIndiceParcial (int indice)
{
    indiceParcial = juce::jmax (1, indice);
    // Ajustar frequência para o harmônico apropriado
    incrementoFase = 2.0f * juce::MathConstants<float>::pi * 
                     (frequenciaHz * indiceParcial) / taxaAmostragem;
}

//==============================================================================
void ModalOscillator::setBrilhoEspectral (float brilho)
{
    brilhoEspectral = juce::jlimit (0.0f, 1.0f, brilho);
    // Harmônicos mais altos são atenuados pelo brilho
    float atenuacao = std::pow (brilhoEspectral, indiceParcial / 32.0f);
    amplitude = amplitudeAlvo * atenuacao;
}

//==============================================================================
float ModalOscillator::processar()
{
    aplicarEnvelopeModal();
    float amostra = gerarSenoideModal();
    
    // Incrementar fase
    fase += incrementoFase;
    if (fase > 2.0f * juce::MathConstants<float>::pi)
        fase -= 2.0f * juce::MathConstants<float>::pi;
    
    return amostra;
}

//==============================================================================
void ModalOscillator::reset()
{
    fase = 0.0f;
    amplitude = 0.0f;
    amplitudeAlvo = 0.0f;
    envolvente = 1.0f;
}

//==============================================================================
bool ModalOscillator::estaInativo() const
{
    return amplitude < 0.001f && amplitudeAlvo < 0.001f;
}

//==============================================================================
float ModalOscillator::gerarSenoideModal()
{
    // Gerar senoide com amplitude modulada pelo envelope
    float sinal = std::sin (fase) * amplitude * envolvente;
    return sinal;
}

//==============================================================================
void ModalOscillator::aplicarEnvelopeModal()
{
    // Suavizar mudanças de amplitude (smoothing)
    float suavizacao = 0.05f;
    amplitude += (amplitudeAlvo - amplitude) * suavizacao;
    
    // Aplicar decaimento exponencial (amortecimento modal)
    envolvente *= taxaDecaimento;
    
    // Evitar valores muito pequenos
    if (envolvente < 0.00001f)
        envolvente = 0.0f;
}

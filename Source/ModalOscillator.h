/*
  ==============================================================================

    Oscilador Modal para Síntese Espectral
    Implementa ressintetização com amortecimento modal

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

//==============================================================================
/**
    Oscilador modal individual para síntese aditiva
    Gera harmônicos com envelope de amortecimento
*/
class ModalOscillator
{
public:
    ModalOscillator() = default;
    ~ModalOscillator() = default;

    //==============================================================================
    // Inicializar oscilador para uma nota MIDI
    void inicializar (int notaMidi, float taxaAmostragem);
    
    // Configurar amplitude do parcial
    void setAmplitude (float amp);
    
    // Configurar fator de qualidade (amortecimento)
    void setQFactor (float q);
    
    // Configurar índice do parcial (1 = fundamental, 2 = 2º harmônico, etc.)
    void setIndiceParcial (int indice);
    
    // Definir brilho espectral (atenua altas frequências)
    void setBrilhoEspectral (float brilho);
    
    //==============================================================================
    // Gerar próxima amostra
    float processar();
    
    // Resetar estado do oscilador
    void reset();
    
    //==============================================================================
    // Verificar se o oscilador está inativo
    bool estaInativo() const;

private:
    //==============================================================================
    // Estado do oscilador
    float fase = 0.0f;
    float incrementoFase = 0.0f;
    float amplitude = 0.0f;
    float amplitudeAlvo = 0.0f;
    
    // Parâmetros modais
    int indiceParcial = 1;
    float qFactor = 0.95f;              // Amortecimento
    float brilhoEspectral = 1.0f;
    
    // Envelope ADSR simplificado para modal
    float envolvente = 1.0f;
    float taxaDecaimento = 0.99f;
    
    // Frequência em Hz
    float frequenciaHz = 440.0f;
    float taxaAmostragem = 44100.0f;
    
    //==============================================================================
    // Gerar senoide com modulação de amplitude
    float gerarSenoideModal();
    
    // Aplicar envelope de amortecimento modal
    void aplicarEnvelopeModal();
};

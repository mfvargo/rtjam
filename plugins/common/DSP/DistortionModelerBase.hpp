#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"
#include "SignalBlock.hpp"

class DistortionModelerBase : public Effect
{
public:
  enum HpfMode
  {
    low,
    high,
  };

  void init() override
  {
    // Setup base class stuff
    Effect::init();
   
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();
    
    setupFilters();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      // HPF in chain - determines amount of low-freqs sent to clipper block
      // use F~140 Hz for full-range. F=720 for Tubescreamer type distorion
      float value = m_hpf1.getSample(input[i]);
      value = m_lpf1.getSample(value); 

      // Stage 1 - first clipper (set to emulate op-amp clipping before diodes)
      value = SignalBlock::clipSample(m_clip1Type, value * m_gain1); // clip signal
                            // filter out higher-order harmonics
      value = m_lpf2.getSample(value); 
      value = m_hpf2.getSample(value);

      // Stage 2 - second clipper option if m_stages == 2
      if(m_stages == 2)
      {
        value = m_hpf2.getSample(value);  
        value = SignalBlock::clipSample(m_clip2Type, value * m_gain2);
        value = m_lpf3.getSample(value); // filter out higher-order harmonics
        value = m_hpf3.getSample(value);
      }  
      // Stage 3 - Tone control - 3 band EQ - low shelf, mid cut/boost, high shelf
      // Baxandall type w/ mid control
      //
      // TODO: I don't think you want to stack these.  I think you want to add the three onto the original
      // See ToneStack.hpp lines 73-75
      // COMMENT: this is ok and by design - tonestack uses parallel structure with gains on fixed filters that sum together
      // this is 3 band eq with series cascade structure and the freq and cut/boost will be varied by
      // just recalculating the filter coeffs vs a gain constant like in TS
      //
      value = m_toneBass.getSample(value);
      value = m_toneMidrange.getSample(value);
      value = m_toneTreble.getSample(value);

      // sum in some dry level for detail (to model Klon and similar pedals)

      output[i] = (value * m_level) + (m_dryLevel * input[i]);
    }
  };

private:
  BiQuadFilter m_hpf1; // pre-clipping high-pass filter
  BiQuadFilter m_hpf2; // post-clip 1 high-pass filter
  BiQuadFilter m_hpf3; // post-clip 2 high-pass filter

  BiQuadFilter m_lpf1; // pre-clip 1 low-pass filter 
  BiQuadFilter m_lpf2; // post clip 1 low pass-filter
  BiQuadFilter m_lpf3; // post clip 1 low pass-filter

  BiQuadFilter m_upsample;   // upsample filter - runs at 8 * 48k after upsample
  BiQuadFilter m_downsample; // downsample filter - runs at 8 * 48k after clip and before downsample

  BiQuadFilter m_toneBass;     // bass control frequency
  BiQuadFilter m_toneMidrange; // mid control frequency
  BiQuadFilter m_toneTreble;   // treble control frequency

  float m_filterOut[8];
  float m_upsampleBuffer[8];
  float m_clipOut[8];


protected:
  // Parameters
  SignalBlock::ClipType m_clip1Type; // clip funciton type for stage 1
  SignalBlock::ClipType m_clip2Type; // clip funciton type for stage 2

  float m_gain1;                     // gain before clip functions
  float m_gain2;                     // gain before clip functions
  
  unsigned int m_stages = 2;  // not used yet...

  float m_level;                    // Overall level

  float m_lpf1Freq = 8000; // frequency of the first clip block lpf (fixed filter)
  float m_lpf2Freq = 5000; // frequency of the first clip block lpf (fixed filter)
  float m_lpf3Freq = 5000; // frequency of the first clip block lpf (fixed filter)
  
  float m_hpf1Freq = 250;   // high-pass filter 1 cutoff
  float m_hpf2Freq = 30;   // high-pass filter 1 cutoff
  float m_hpf3Freq = 30;   // high-pass filter 1 cutoff

  float m_toneBassFreq;     // LPF cut-off frequency for tone control
  float m_toneMidrangeFreq; // HPF cut-off frequency for tone control
  float m_toneTrebleFreq;   // HPF cut-off frequency for tone control

  float m_toneBassCutBoost;     // LPF cut-off frequency for tone control
  float m_toneMidrangeCutBoost; // HPF cut-off frequency for tone control
  float m_toneTrebleCutBoost;   // HPF cut-off frequency for tone control

  float m_dryLevel; // amount of dry to add in at end of chain
                    // (to model Klon type drives or add detail to high gain model)

  void setupFilters()
  {

    // TODO - low-pass and highpass don't have cut/boost param - set to 0 (it's ignored in the coeff calcs)
    // Setup filters
    m_hpf1.init(BiQuadFilter::FilterType::HighPass, m_hpf1Freq, 1.0, 1.0, 48000);
    m_hpf2.init(BiQuadFilter::FilterType::HighPass, m_hpf2Freq, 1.0, 1.0, 48000);
    m_hpf3.init(BiQuadFilter::FilterType::HighPass, m_hpf3Freq, 1.0, 1.0, 48000);
    m_lpf1.init(BiQuadFilter::FilterType::LowPass, m_lpf1Freq, 1.0, 1.0, 48000);
    m_lpf2.init(BiQuadFilter::FilterType::LowPass, m_lpf2Freq, 1.0, 1.0, 48000);
    m_lpf3.init(BiQuadFilter::FilterType::LowPass, m_lpf3Freq, 1.0, 1.0, 48000);
  
    m_upsample.init(BiQuadFilter::FilterType::LowPass, 48000, 1.0, 1.0, 8 * 384000);
    m_downsample.init(BiQuadFilter::FilterType::LowPass, 48000, 1.0, 1.0, 8 * 384000);
    m_toneBass.init(BiQuadFilter::FilterType::LowShelf, m_toneBassFreq, m_toneBassCutBoost, 0.707, 48000);
    m_toneMidrange.init(BiQuadFilter::FilterType::Peaking, m_toneMidrangeFreq, m_toneMidrangeCutBoost, 0.707, 48000);
    m_toneTreble.init(BiQuadFilter::FilterType::HighShelf, m_toneTrebleFreq, m_toneTrebleCutBoost, 0.707, 48000);
  };

  float distortionAlgorithm(float inputSample, float distortionGain, SignalBlock::ClipType distortionClipType)
  {

    // upsample to a vector 8x the size (zero padded)
    for (int i = 0; i < 8; i++)
    {
      if (i == 0)
      {
        m_upsampleBuffer[i] = inputSample;
      }
      else
      {
        m_upsampleBuffer[i] = 0;
      }
    }

    // filter the vector
    for (int i = 0; i < 8; i++)
    {
      // filter at upsampled Fs/8 - 8th order filter
      m_filterOut[i] = m_upsample.getSample(m_upsampleBuffer[i]);
      m_filterOut[i] = m_upsample.getSample(m_filterOut[i]);
      m_filterOut[i] = m_upsample.getSample(m_filterOut[i]);
      m_filterOut[i] = m_upsample.getSample(m_filterOut[i]);

      // add gain to signal and clip
      // using either hard, soft, asymmetric or symmetric clipper
      m_filterOut[i] = 8 * distortionGain * m_filterOut[i];
      m_clipOut[i] = SignalBlock::clipSample(distortionClipType, m_filterOut[i]);

      // filter at upsampled Fs/8 before downsampling - 8th order filter
      m_clipOut[i] = m_downsample.getSample(m_clipOut[i]);
      m_clipOut[i] = m_downsample.getSample(m_clipOut[i]);
      m_clipOut[i] = m_downsample.getSample(m_clipOut[i]);
      m_clipOut[i] = m_downsample.getSample(m_clipOut[i]);
    }

    // down-sample the vector back to original sample rate
    return m_clipOut[0];
  };
};

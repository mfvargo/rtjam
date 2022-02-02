#pragma once

#include "json.hpp"
#include "Effect.hpp"
#include "Delay.hpp"
#include "MonoVerb.hpp"
#include "Distortion.hpp"
#include "DistortionModeler.hpp"
#include "Tremelo.hpp"
#include "ToneStack.hpp"
#include "Chorus.hpp"
#include "BassDI.hpp"
#include "NoiseGate.hpp"
#include "SigmaReverb.hpp"
#include "SpeakerSimIIR.hpp"
#include "Compressor.hpp"
#include "BassEnvelope.hpp"
#include "GuitarEnvelope.hpp"
#include "DistortionSuperModeler.hpp"
#include "DistortionTubeDrive.hpp"
#include "DistortionSoulDrive.hpp"
#include "DistortionMetalDrive.hpp"
#include "DistortionOctaveFuzz.hpp"
#include "Boost.hpp"

using json = nlohmann::json;
using namespace std;

// this maps the short name to long name for the pedal
// PLEASE NOTE:  The first field (key) MUST match the m_name value
// for the class it will manufacture.  For example, if the key is "TubeDrive", then
// the value of m_name set by that class must match!  otherwise the saved json
// won't match and the factory won't be able to create the pedal.
const static json s_PedalTypes = {
    {"Bass DI", "Bass Guitar Tone Shaping"},
    {"DistortionModeler", "Distortion Playground"},
    {"Distortion", "Distortion Overdrive"},
    {"Delay", "Delay Pedal"},
    {"Chorus", "Chorus"},
    {"Reverb", "Reverb"},
    {"Tremelo", "Tremelo ala Fender"},
    {"Tone Stack", "Tone controls (3 band)"},
    {"Noise Gate", "Noise Gate"},
    {"Sigma Reverb", "Sigma Reverb"},
    {"Speaker Sim", "Speaker Cabinet Simulator"},
    {"Compressor", "Compressor Pedal"},
    {"Bass Envelope", "Bass Envelope Filter Pedal"},
    {"Guitar Envelope", "Guitar Envelope Filter Pedal (auto-wah)"},
    {"DistortionSuperModeler", "Distortion Super Modeler"},
    {"TubeDrive", "Tube Overdrive"},
    {"SoulDrive", "Soul Overdrive"},
    {"MetalDrive", "Metal Overdrive"},
    {"OctaveFuzz", "Octave Fuzz"},
    {"Boost", "Boost Pedal"}};

class EffectFactory
{
public:
  static Effect *manufacture(json effect)
  {
    try
    {
      // Step 1:  Figure out what kind of effect to manufacture
      Effect *rval = NULL;
      if (effect["name"] == "DistortionModeler")
      {
        rval = new DistortionModeler();
      }
      else if (effect["name"] == "Distortion")
      {
        rval = new Distortion();
      }
      else if (effect["name"] == "Bass DI")
      {
        rval = new BassDI();
      }
      else if (effect["name"] == "Delay")
      {
        rval = new SigmaDelay();
      }
      else if (effect["name"] == "Chorus")
      {
        rval = new Chorus();
      }
      else if (effect["name"] == "Reverb")
      {
        rval = new MonoVerb();
      }
      else if (effect["name"] == "Tremelo")
      {
        rval = new Tremelo();
      }
      else if (effect["name"] == "Tone Stack")
      {
        rval = new ToneStack();
      }
      else if (effect["name"] == "Noise Gate")
      {
        rval = new NoiseGate();
      }
      else if (effect["name"] == "Sigma Reverb")
      {
        rval = new SigmaReverb();
      }
      else if (effect["name"] == "Speaker Sim")
      {
        rval = new SpeakerSimIIR();
      }
      else if (effect["name"] == "Compressor")
      {
        rval = new Compressor();
      }
      else if (effect["name"] == "Bass Envelope")
      {
        rval = new BassEnvelope();
      }
      else if (effect["name"] == "Guitar Envelope")
      {
        rval = new GuitarEnvelope();
      }
      else if (effect["name"] == "DistortionSuperModeler")
      {
        rval = new DistortionSuperModeler();
      }
      else if (effect["name"] == "TubeDrive")
      {
        rval = new DistortionTubeDrive();
      }
      else if (effect["name"] == "SoulDrive")
      {
        rval = new DistortionSoulDrive();
      }
      else if (effect["name"] == "MetalDrive")
      {
        rval = new DistortionMetalDrive();
      }
      else if (effect["name"] == "OctaveFuzz")
      {
        rval = new DistortionOctaveFuzz();
      }
      else if (effect["name"] == "Boost")
      {
        rval = new Boost();
      }

      else
      {
        return rval;
      }
      // Step 2, initialize the effect
      rval->init();

      for (auto &setting : effect["settings"])
      {
        rval->setSettingValue(setting);
      }
      return rval;
    }
    catch (...)
    {
      return NULL;
    }
  }
};

// This is the sample json for the Distortion pedal
// {
//   "index" : 0,
//   "name" : "Distortion",
//   "settings" : [
//     {"labels" : [], "max" : 1, "min" : 0, "name" : "bypass", "step" : 1, "type" : 2, "units" : 4, "value" : true},
//     {"labels" : [ "hard", "soft", "asym", "even" ], "max" : 3, "min" : 0, "name" : "clipType", "step" : 1, "type" : 1, "units" : 3, "value" : 1},
//     {"labels" : [], "max" : 40, "min" : 0, "name" : "drive", "step" : 0.5, "type" : 0, "units" : 1, "value" : 6},
//     {"labels" : [ "hard", "soft", "asym", "even" ], "max" : 10, "min" : -30, "name" : "level", "step" : 0.5, "type" : 0, "units" : 1, "value" : 0}
//   ]
// }
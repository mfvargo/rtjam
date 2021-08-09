#include "json.hpp"
#include "Effect.hpp"
#include "Delay.hpp"
#include "HighPassFilter.hpp"
#include "MonoVerb.hpp"
#include "Distortion.hpp"
#include "DistortionModeler.hpp"
#include "Tremelo.hpp"
#include "ToneStack.hpp"
#include "Chorus.hpp"
#include "BassDI.hpp"
#include "SpeakerSimIIR.hpp"

using json = nlohmann::json;
using namespace std;

const static json s_PedalTypes = {
    {"Bass DI", "Bass Guitar Tone Shaping"},
    {"DistortionModeler", "Distortion Playground"},
    {"Distortion", "Distortion Overdrive"},
    {"Delay", "Delay Pedal"},
    {"Chorus", "Chorus"},
    {"Reverb", "Reverb"},
    {"Tremelo", "Tremelo ala Fender"},
    {"Tone Stack", "Tone controls (3 band)"},
    {"DC Offset HighPass", "DC removal Filter"},
    {"Speaker Sim", "Speaker Cabinet Simulator"}
    };

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
      else if (effect["name"] == "DC Offset HighPass")
      {
        rval = new HighPassFilter();
      }
      else if (effect["name"] == "Speaker Sim")
      {
        rval = new SpeakerSimIIR();
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
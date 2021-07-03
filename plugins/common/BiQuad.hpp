#pragma once

#include "SignalBlock.hpp"

class BiQuadFilter : public SignalBlock
{
public:
  void init(){

  };

  json getConfig() override
  {
    json config = {};
    return config;
  }

  float getSample() override
  {
    return 0.0;
  };
};
#ifndef JAM_METER_BAR_HPP_INCLUDED
#define JAM_METER_BAR_HPP_INCLUDED

#include "ImageWidgets.hpp"
#include "PartialImage.hpp"

namespace JamNetStuff
{

  class JamMeterBar {
    public:
      JamMeterBar();

      void drawAt(Point<int> pos, int height, float level);

    private:
      PartialImage fLitUp;
      PartialImage fDark;
  };

};
#endif //JAM_METER_BAR_HPP_INCLUDED
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Effect.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

using namespace yarp::os;
using namespace yarp::sig;

#include "EdgeMan.h"

/**
 *
 * An effect that adds a dove, heart, and bouquet.
 *
 */
class EngageEffect : public Effect {
  virtual bool draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2);

  virtual std::string getName() {
    return "EngageTV";
  }
};

Effect *engageRegister() {
  return new EngageEffect();
}

bool EngageEffect::draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2) {
  static EdgeMan edgy;
  edgy.Decorate(src2,dest2);
  return true;
}

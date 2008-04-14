
#ifndef YEFFECTS_INC
#define YEFFECTS_INC

#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/dev/DeviceDriver.h>

#include <string>
#include <vector>

#include "Effect.h"


class EffectGroup {
private:
  std::vector <Effect *> effects;
  void add(Effect *effect);
public:
  EffectGroup();
  int init();

  Effect *search(const char *str);

  virtual ~EffectGroup() {
    for (int i=0; i<effects.size(); i++) {
      if (effects[i]!=NULL) {
	delete effects[i];
	effects[i] = NULL;
      }
    }
    effects.clear();
  }

  static EffectGroup& get();

  yarp::os::Bottle getList() {
    yarp::os::Bottle bot;
    for (int i=0; i<effects.size(); i++) {
      bot.addString(effects[i]->getName().c_str());
    }
    return bot;
  }
};


// specific effects - should get moved
Effect *engageRegister();
Effect *tickerRegister();
Effect *paramRegister();

#endif





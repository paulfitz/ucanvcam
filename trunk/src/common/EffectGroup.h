
#ifndef YEFFECTS_INC
#define YEFFECTS_INC

#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/dev/DeviceDriver.h>

#include <string>
#include <vector>

#include "Effect.h"


class YarpEffects {
private:
  std::vector <YarpEffect *> effects;
  void add(YarpEffect *effect);
public:
  YarpEffects();
  int init();

  YarpEffect *search(const char *str);

  virtual ~YarpEffects() {
    for (int i=0; i<effects.size(); i++) {
      if (effects[i]!=NULL) {
	delete effects[i];
	effects[i] = NULL;
      }
    }
    effects.clear();
  }

  static YarpEffects& get();

  yarp::os::Bottle getList() {
    yarp::os::Bottle bot;
    for (int i=0; i<effects.size(); i++) {
      bot.addString(effects[i]->getName().c_str());
    }
    return bot;
  }
};



YarpEffect *ypaulfitzRegister();

YarpEffect *ytickerRegister();

YarpEffect *yparamRegister();

#endif





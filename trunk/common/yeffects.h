
#ifndef YEFFECTS_INC
#define YEFFECTS_INC

#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>
#include <yarp/dev/DeviceDriver.h>

#include <string>
#include <vector>

class YarpEffect : public yarp::dev::DeviceDriver {
public:
  virtual bool open(yarp::os::Searchable& config) {
    return true;
  }

  virtual bool close() {
    return true;
  }

  virtual bool draw(yarp::sig::ImageOf<yarp::sig::PixelRgb>& src,
		    yarp::sig::ImageOf<yarp::sig::PixelRgb>& dest) {
    return false;
  }

  virtual yarp::sig::Image *pdraw(yarp::sig::Image& src,
				  yarp::sig::Image& dest) {
    yarp::sig::ImageOf<yarp::sig::PixelRgb> src2;
    yarp::sig::ImageOf<yarp::sig::PixelRgb> dest2;
    bool cpIn = (src.getPixelCode()!=src2.getPixelCode());
    bool cpOut = (dest.getPixelCode()!=dest2.getPixelCode());
    if (cpIn) {
      src2.copy(src);
    } else {
      src2.wrapIplImage(src.getIplImage());
    }
    if (!cpOut) {
      dest.resize(src);
      dest2.wrapIplImage(dest.getIplImage());
    }
    if (!draw(src2,dest2)) {
      return NULL;
    }
    if (cpOut) {
      dest.copy(dest2);
    }
    return &dest;
  }

  /*
  virtual bool draw(yarp::sig::ImageOf<yarp::sig::PixelRgba>& src,
		    yarp::sig::ImageOf<yarp::sig::PixelRgba>& dest) {
    return false;
  }
  */

  virtual std::string getName() {
    return "anon";
  }

  virtual ~YarpEffect() {
  }

  virtual bool start() {
  }

  virtual bool stop() {
  }
};


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





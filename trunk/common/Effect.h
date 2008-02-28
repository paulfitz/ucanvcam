#ifndef EFFECT_INC
#define EFFECT_INC

// put an API around all the messy code, which should then
// get cleaned up step-by-step

#include "yeffects.h"
#include "yarpy.h"

class Effect {
private:
public:
  Effect() {
  }

  virtual ~Effect() {
    stop();
  }

  yarp::os::Bottle getEffects() {
    return YarpEffects::get().getList();
  }

  bool setEffect(const char *name) {
    YarpEffect *e = YarpEffects::get().search(name);
    if (e==NULL) {
      return false;
    }
    yarpy_set_effect(e);
    return true;
  }

  bool apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
	     yarp::sig::ImageOf<yarp::sig::PixelRgb>& out) {
    return yarpy_apply(in,out);
  }

  bool apply(yarp::sig::ImageOf<yarp::sig::PixelBgr>& in, 
	     yarp::sig::ImageOf<yarp::sig::PixelBgr>& out) {
    return yarpy_apply(in,out);
  }

  bool apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
	     yarp::sig::ImageOf<yarp::sig::PixelBgr>& out) {
    return yarpy_apply(in,out);
  }

private:
  void stop() {
    yarpy_stop();
  }
};

#endif


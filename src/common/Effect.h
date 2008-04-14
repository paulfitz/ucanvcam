#ifndef EFFECT_INC
#define EFFECT_INC

// put an API around all the messy code, which should then
// get cleaned up step-by-step

#include "yeffects.h"
#include "yarpy.h"

/**
 *
 * Standard interface to the available effects.
 * The Effect::apply method applies an effect selected using Effect::setEffect.
 * The apply method can be called with various image formats.
 *
 */
class Effect {
private:
public:
  Effect() {
    yarpy_init();
  }

  virtual ~Effect() {
    stop();
  }

  /**
   *
   * Get a list of the available effects.
   *
   */
  yarp::os::Bottle getEffects() {
    return YarpEffects::get().getList();
  }

  /**
   *
   * Choose a named effect.
   *
   */
  bool setEffect(const char *name) {
    YarpEffect *e = YarpEffects::get().search(name);
    if (e==NULL) {
      return false;
    }
    yarpy_set_effect(e);
    return true;
  }

  /**
   *
   * Apply the current effect to an image, RGB version.
   *
   */
  bool apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
	     yarp::sig::ImageOf<yarp::sig::PixelRgb>& out) {
    return yarpy_apply(in,out);
  }

  /**
   *
   * Apply the current effect to an image, BGR version.
   *
   */
  bool apply(yarp::sig::ImageOf<yarp::sig::PixelBgr>& in, 
	     yarp::sig::ImageOf<yarp::sig::PixelBgr>& out) {
    return yarpy_apply(in,out);
  }

  /**
   *
   * Apply the current effect to an image, RGB to BGR version.
   *
   */
  bool apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
	     yarp::sig::ImageOf<yarp::sig::PixelBgr>& out) {
    return yarpy_apply(in,out);
  }

  /**
   *
   * Get the configuration of the current effect.
   *
   */
  yarp::os::Property getConfiguration() {
    YarpEffect *e = yarpy_get_effect();
    if (e!=NULL) {
      return e->getConfiguration();
    }
    return yarp::os::Property();
  }

  /**
   *
   * Reconfigure the current effect.
   *
   */
  void setConfiguration(yarp::os::Property& prop) {
    YarpEffect *e = yarpy_get_effect();
    if (e!=NULL) {
      e->reconfigure(prop);
    }
  }

private:
  void stop() {
    yarpy_stop();
  }
};

#endif



#include <stdio.h>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

extern "C" {
#include "EffecTV.h"
#include "utils.h"
#include "effects.h"
}

#include "EffectGroup.h"
#include "Effects.h"

int video_width;
int video_height;
int video_area;

int screen_width;
int screen_height;
int screen_scale = 1;

int doublebuf = 0;
int stretch = 0;

static Screeny scr;

Screeny *screen = &scr;

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;

#define RAW(x) ((RGB32*)(x).getRawImage())

void screen_setcaption(const char *str) {
  printf("set caption %s\n", str);
}

static Effect *ye = NULL;
static ImageOf<PixelBgra> src, dest;

static void ucanvcam_init() {
  static bool first = true;
  if (first) {
    //ye = effects.search("NervousTV");
    ye = EffectGroup::get().search("TickerTV");
    //ye = EffectGroup::get().search("EngageTV");
    //ye = EffectGroup::get().search("BrokenTV");
  }
  first = false;
}


static void ucanvcam_start(Image& in) {
  static bool first = true;
  if (first) {
    bool cp = false;
    if (in.getPixelCode()!=src.getPixelCode()) {
      src.copy(in);
      cp = true;
    }
      video_width = in.width();
      video_height = in.height();
      video_area = video_width*video_height;
    
      screen_width = in.width();
      screen_height = in.height();
    
      screen->pixels = RAW(cp?src:in);
      //screen_init(video_width,video_height,1);
      image_init();
      //palette_init();
      sharedbuffer_init();

#ifdef WIN32
#else
      //      ye = EffectGroup::get().search("ParamTV");
#endif
      //ye = EffectGroup::get().search("BrokenTV");

      if (ye!=NULL) {
	ye->start();
      }

  }
  first = false;
}


static void ucanvcam_init(Image& in) {
  ucanvcam_init();
  ucanvcam_start(in);
}


static bool ucanvcam_apply(ImageOf<PixelRgb>& in, ImageOf<PixelRgb>& out) {
  ucanvcam_init(in);
  src.copy(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

static bool ucanvcam_apply(ImageOf<PixelBgr>& in, ImageOf<PixelBgr>& out) {
  ucanvcam_init(in);
  src.copy(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

static bool ucanvcam_apply(ImageOf<PixelBgra>& in, ImageOf<PixelBgra>& out) {
  ucanvcam_init(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

static bool ucanvcam_apply(ImageOf<PixelRgb>& in, ImageOf<PixelBgr>& out) {
  ucanvcam_init(in);
  src.copy(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

static bool ucanvcam_stop() {

  if (ye!=NULL) {
    ye->stop();
  }

  //screen_quit();
  sharedbuffer_end();
  //palette_end();
  image_end();

  return true;
}

static Effect *ucanvcam_get_effect() {
  return ye;
}

static Effect *ucanvcam_take_effect() {
  if (ye!=NULL) {
    ye->stop();
  }
  Effect *tmp = ye;
  ye = NULL;
  return tmp;
}

Effects::Effects() {
  ucanvcam_init();
}

yarp::os::Bottle Effects::getEffects() {
  return EffectGroup::get().getList();
}

bool Effects::setEffect(const char *name) {
  Effect *next = EffectGroup::get().search(name);
  if (next==NULL) {
    return false;
  }
  if (ye!=NULL) {
    ye->stop();
    ye = NULL;
  }
  ye = next;
  if (ye!=NULL) {
    ye->start();
  }
  return true;
}


bool Effects::apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
	   yarp::sig::ImageOf<yarp::sig::PixelRgb>& out) {
  return ucanvcam_apply(in,out);
}

bool Effects::apply(yarp::sig::ImageOf<yarp::sig::PixelBgr>& in, 
		   yarp::sig::ImageOf<yarp::sig::PixelBgr>& out) {
  return ucanvcam_apply(in,out);
}

bool Effects::apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
		   yarp::sig::ImageOf<yarp::sig::PixelBgr>& out) {
  return ucanvcam_apply(in,out);
}

yarp::os::Property Effects::getConfiguration() {
  Effect *e = ucanvcam_get_effect();
  if (e!=NULL) {
    return e->getConfiguration();
  }
  return yarp::os::Property();
}

void Effects::setConfiguration(yarp::os::Property& prop) {
  Effect *e = ucanvcam_get_effect();
  if (e!=NULL) {
    e->reconfigure(prop);
  }
}

void Effects::stop() {
  ucanvcam_stop();
}

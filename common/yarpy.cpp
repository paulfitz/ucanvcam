#include <stdio.h>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

extern "C" {
#include "EffecTV.h"
#include "utils.h"
#include "effects.h"
}

#include "yarpy.h"
#include "yeffects.h"

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

static YarpEffect *ye = NULL;
static ImageOf<PixelBgra> src, dest;

void yarpy_init(Image& in) {
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

      //ye = effects.search("NervousTV");
      //ye = YarpEffects::get().search("TickerTV");
      ye = YarpEffects::get().search("EngageTV");

#ifdef WIN32
#else
      //      ye = YarpEffects::get().search("ParamTV");
#endif
      //ye = YarpEffects::get().search("BrokenTV");

      if (ye!=NULL) {
	ye->start();
      }

  }
  first = false;
}


bool yarpy_apply(ImageOf<PixelRgb>& in, ImageOf<PixelRgb>& out) {
  yarpy_init(in);
  src.copy(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

bool yarpy_apply(ImageOf<PixelBgr>& in, ImageOf<PixelBgr>& out) {
  yarpy_init(in);
  src.copy(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

bool yarpy_apply(ImageOf<PixelBgra>& in, ImageOf<PixelBgra>& out) {
  yarpy_init(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

bool yarpy_apply(ImageOf<PixelRgb>& in, ImageOf<PixelBgr>& out) {
  yarpy_init(in);
  src.copy(in);
  if (ye!=NULL) {
    Image *img = ye->pdraw(in,out);
    if (img!=&out) {
      out.copy(*img);
    }
  }
}

bool yarpy_stop() {

  if (ye!=NULL) {
    ye->stop();
  }

  //screen_quit();
  sharedbuffer_end();
  //palette_end();
  image_end();

  return true;
}


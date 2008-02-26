#include <stdio.h>
#include <assert.h>

#include "EdgeMan.h"
#include "ImgTrack.h"

#include "Sailor.h"


#define SCT (3)

float xRing = 0, yRing = 0;
int ctRing = 0;

void fix(double& target, int val, int dummy) {
  if (target<val) target = val;
}

void EdgeMan::Decorate(YARPImageOf<YarpPixelRGB>& src,
		       YARPImageOf<YarpPixelRGB>& dest) {

  static YARPImageOf<YarpPixelRGB> view, view2;
  static Sailor sailor[SCT];
  static int first = 1;
  if (first) {
    printf("Loading images...\n");
    sailor[0].put(64,64,50,5,0.75,0.08,11);
    sailor[0].attach("test.ppm");
    sailor[1].put(700,64,-80,0.0,0.0,0.0,7);
    sailor[1].attach_many("dove_%03d.ppm",18);
    sailor[2].put(300,300,-150,-0.1,0.7,0.08,9);
    sailor[2].attach("rose.ppm");
    first = 0;
  }
  dest = src;


  static YARPImageTrackTool tt;
  tt.Apply(src);
  int xxRing = tt.GetX();
  int yyRing = tt.GetY();

  static int px = xxRing;
  static int py = yyRing;
  static int pct = 0;
  if (fabs(xxRing-px)>1 || fabs(yyRing-py)>1) {
    if (pct>2 && pct<20) {
      pct = 20;
    }
    pct++;
  } else {
    if (pct>0) {
      pct--;
    }
  } 
  if (pct>100) {
    pct = 100;
  }
  px = xxRing;
  py = yyRing;

  ctRing = pct;

  int veer = 0;

  if (ctRing>5) {
    veer = 1;
  }

  veer = 1;

  double fx = 0;
  double fy = 0;
  double fct = 0;
  int fi = 0;
  IMGFOR(src,x,y) {
    PixelRgb& pix = src(x,y);
    if (pix.r>1.2*pix.g && pix.r>1.2*pix.b) {
      float scale = 1-(y/((double)src.height()));
      scale *= pix.r;
      fx+=x*scale;
      fy+=y*scale;
      fct+=scale;
      fi++;
    }
  }
  if (fct>0.0001) {
    fx /= fct;
    fy /= fct;
  }
  if (fi>20) {
    tt.SetXYScaled((int)fx,(int)fy);
  }



  int mind = 20;
  int maxd = mind*mind;
  /*
  if (veer)
  for (int x=xxRing-mind; x<=xxRing+mind; x++) {
    for (int y=yyRing-mind; y<=yyRing+mind; y++) {
      double d = (x-xxRing)*(x-xxRing) + (y-yyRing)*(y-yyRing);
      if (d<maxd) {
	double sc = (maxd-d)/maxd + 1;
	if (1) { 
	  YarpPixelRGB& pix = dest.SafePixel(x,y);
	  int rr = pix.r;
	  int gg = pix.g;
	  int bb = pix.b;
	  rr *= sc;
	  gg *= sc;
	  bb *= sc;
	  if (rr>255) rr = 255;
	  if (gg>255) gg = 255;
	  if (bb>255) bb = 255;
	  pix.r = rr;
	  pix.g = gg;
	  pix.b = bb;
	}
      }
    }
  }
  */


  if (veer) {
    YarpPixelRGB pixwhite(255,255,255);
  } else {
  }
  if (!veer) {
    xxRing = dest.GetWidth()/2;
    yyRing = dest.GetWidth()/2;
  }
  for (int i=0; i<SCT; i++) {
    sailor[i].update();
    sailor[i].veer(xxRing,yyRing,1);
    sailor[i].fade(veer);
    sailor[i].show(dest);
  }

  double now = Time::now();
  static double tfirst = now;
  float ratio = 12 + 4*sin(2*(now-tfirst));

  int bit = (int)(src.width()/ratio);
  //printf("bit is %d / %g %g\n", bit, ratio, now-tfirst);
  IMGFOR(dest,x,y) {
    double del = -1;
    int con = 0;
    if (x<bit) { fix(del,bit-x,con); }
    if (y<bit) { fix(del,bit-y,con); }
    if (x>=src.width()-bit) { fix(del,x-(src.width()-bit),con); }
    if (y>=src.height()-bit) { fix(del,y-(src.height()-bit),con); }
    if (del>=-0.5) {
      PixelRgb& pix = dest(x,y);
      pix.r += (256-pix.r)*(del/bit);
    }
  }
}




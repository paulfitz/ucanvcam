#ifndef SAILOR_INC
#define SAILOR_INC


#include <yarp/os/all.h>
#include <yarp/sig/all.h>
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;
using namespace yarp::sig::draw;

#include "location.h"

#define YARPImageOf ImageOf
#define YarpPixelRGB PixelRgb
#define GetWidth width
#define GetHeight height
#define SafePixel safePixel
#define AddCrossHair addCrossHair

#define MAXIMG (20)

class Sailor {
public:
  YARPImageOf<YarpPixelRGB> imgs[MAXIMG];
  YARPImageOf<YarpPixelRGB> img;
  int index;
  double x, y, vx, vy, vmag, sgn, vtheta, scale, vscale, mscale;
  double dist;
  double fader, fader_target, theta;

  Sailor() {
    fader = 0;
    fader_target = 1;
    sgn = 1;
    theta = 0;
    vtheta = 0.08;
    scale = 0.01;
    vscale = 5;
    mscale = 0.5;
    index = 0;
  }

  void put(double ix, double iy, double ndist, double beat, double mbeat,
	   double nvtheta = 0.08, double nvmag = 9) {
    x = ix; 
    y = iy;
    vx = 1;
    vy = 0;
    vmag = nvmag;
    dist = fabs(ndist)*0.1;
    sgn = (ndist<0)?-1:1;
    vscale = beat;
    mscale = mbeat*0.5;
    vtheta = nvtheta;
  }

  void fade(int show = 0) {
    fader_target = show;
  }

  void attach(const char *filename) {
    attachImg(img,filename);
  }

  void attachImg(ImageOf<PixelRgb>& target, const char *filename) {
    char buf[1000];
    printf("Loading %s\n",filename);
    sprintf(buf,"%s/%s",getResourceLocation().c_str(),filename);
    read(target,buf);
  }

  void attach_many(const char *filename, int ct) {
    for (int i=0; i<ct && i<MAXIMG; i++) {
      char buf[256];
      sprintf(buf,filename,i);
      attachImg(imgs[i],buf);
    }
  }

  void show(YARPImageOf<YarpPixelRGB>& dest) {
    int flip = 1;
    if (imgs[0].GetWidth()!=0) {
      img = imgs[index];
      index++;
      if (imgs[index].GetWidth()==0) {
	index = 0;
      }
      if (vx<0) {
	flip = -1;
      }
    }
    if (img.GetWidth()!=0) {
      int x0 = getX(); //+img.GetWidth()/2;
      int y0 = getY(); //+img.GetHeight()/2;
      YarpPixelRGB pix0(255,255,255);
      YarpPixelRGB pixn = img(img.GetWidth()-1,img.GetHeight()-1);
      //YarpPixelRGB pixn = img(0,0);
      img.SafePixel(-1,-1) = pixn;
      double c = cos(theta);
      double s = sin(theta);
      int x2 = img.GetWidth()/2;
      int y2 = img.GetHeight()/2;
      int rad = (int)(sqrt(((double)(x2*x2 + y2*y2)))+0.5);
      for (int x=-rad; x<=+rad; x++) {
	for (int y=-rad; y<=+rad; y++) {
	  double xx = (x*c-y*s)/scale;
	  double yy = (x*s+y*c)/scale;
	  YarpPixelRGB& pix = img.SafePixel((int)(xx+0.5)+x2,(int)(yy+0.5)+y2);
	  //YarpPixelRGB& pix = img.SafePixel(x,y);
	  //YarpPixelRGB& target = dest.SafePixel(x+x2,y+y2);
	  if (pix.r!=pixn.r || pix.b!=pixn.b) {
//(pix.r<250||pix.b<250)&&) {
	    YarpPixelRGB& target = dest.SafePixel(x0+x*flip,y0+y);
	    target.r = fader*pix.r + (1-fader)*target.r;
	    target.g = fader*pix.g + (1-fader)*target.g;
	    target.b = fader*pix.b + (1-fader)*target.b;
	  }
	}
      }
      //YarpPixelRGB pixwhite(255,255,255);
      //AddCrossHair(dest,pixwhite,getX(),getY(),10);
    } else {
      YarpPixelRGB pixwhite(255,255,255);
      AddCrossHair(dest,pixwhite,getX(),getY(),10);
    }
  }

  void veer(double px, double py, double attraction = 1) {
    double dx = px-x;
    double dy = py-y;
    double mag = sqrt(dx*dx+dy*dy);
    double power = 0;
    if (mag>0.01) { 
      dx /= mag; dy /= mag; 
      power = 1/(mag*mag);
    }
    double ddist = mag-dist;
    vx += 0.01*ddist*dx+0.05*dy*sgn;
    vy += 0.01*ddist*dy-0.05*dx*sgn;
    double vn = sqrt(vx*vx+vy*vy);
    if (vn>0.0001) {
      vx /= vn;
      vy /= vn;
    }
  }

  void update() {
    float factor = 1;
#ifdef TAP_RETURN
    factor = 0.5;
#endif
    x += vx*vmag*factor;
    y += vy*vmag*factor;
    theta += vtheta*factor;
    fader = 0.1*fader_target + 0.9*fader;
    scale = 1-((1+sin(theta*vscale))/2)*mscale;
    scale *= 0.5;
  }

  int getX() { return (int)(x+0.5); }
  int getY() { return (int)(y+0.5); }
};






#endif


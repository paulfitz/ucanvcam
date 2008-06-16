#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Effect.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

#include <string>

#include <ImageLoader.h>
#include "location.h"
#include "MapCache.h"

using namespace yarp::os;
using namespace yarp::sig;

using namespace std;



/**
 *
 * An effect that uses a compiled still/animation as a template.
 *
 */
class PicmixEffect : public Effect {
public:
  PicmixEffect() {
    readDir = "";
    readVersion = "0";
    options.put("dir",
		(getResourceLocation()+"/"+"pm").c_str());
    options.put("version",0);
    needRead = false;
    reconfigure(options);
    first = last = current = 1;
    step = 1;
    linger = 0;
    currentLinger = 0;
  }

  virtual bool draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2);

  virtual std::string getName() {
    return "PicMixTV";
  }

  virtual bool reconfigure(yarp::os::Searchable& config) {
    options.fromString(config.toString());
    if (options.check("dir")) {
      workDir = options.check("dir",Value("picmix")).asString().c_str();
    }
    if (options.check("version")) {
      workVersion = options.check("version",Value("")).toString().c_str();
    }
    needRead = true;
    return true;
  }

  virtual Property getConfiguration() {
    return options;
  }

  void readEffectData();

private:
  int first, last, current, step, linger, currentLinger;
  string workDir, readDir;
  string workVersion, readVersion;
  Property options;
  Property effectConfig;
  bool needRead;
  ImageSeq iseq, xseq, yseq;
};

Effect *picmixRegister() {
  return new PicmixEffect();
}

bool PicmixEffect::draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2) {
  if (needRead) {
    readEffectData();
    needRead = false;
  }
  dest2 = src2;

  //printf("At %d\n", current);

  iseq.go(current);
  ImageLoader *pbase = iseq.get();
  if (pbase==NULL) {
    printf("Failed to get template\n");
    return false;
  }

  xseq.go(current);
  ImageLoader *px = xseq.get();
  if (px==NULL) {
    printf("Failed to get x cache\n");
    dest2 = src2;
    return false;
  }

  yseq.go(current);
  ImageLoader *py = yseq.get();
  if (px==NULL) {
    printf("Failed to get y cache\n");
    return false;
  }

  currentLinger++;
  if (currentLinger>=linger) {
    current+=step;
    if (current>last) current = first;
    currentLinger = 0;
  }

  ImageLoader& img = *pbase;
  int ww = src2.width();
  int hh = src2.height();
  if (pbase->width()<ww) ww = pbase->width();
  if (pbase->height()<hh) hh = pbase->height();

  for (int x=0; x<ww; x++) {
    for (int y=0; y<hh; y++) {
      if (px->isValid()&&py->isValid()) {
	PixelBgra cx = px->pixel(x,y);
	PixelBgra cy = py->pixel(x,y);
	MapCache cache(cx,cy);
	int idx = cache.getIndex()-1;
	if (idx>=0) {
	  //int ox = x;
	  //int oy = y;
	  int ox = int(0.5+(cache.getX()/cache.getScale())*ww);
	  int oy = int(0.5+(cache.getY()/cache.getScale())*ww-(ww-hh)/2);
	  
	  if (ox>=0&&oy>=0&&ox<src2.width()&&oy<src2.height()) {
	    dest2(x,y) = src2.safePixel(ox,oy);
	  } else {
	    PixelBgra ref = pbase->pixel(x,y);
	    dest2(x,y) = PixelRgb(ref.r,ref.g,ref.b);
	  }
	  //dest2(x,y) = PixelRgb(0,0,0);
	} else {
	  PixelBgra ref = pbase->pixel(x,y);
	  dest2(x,y) = PixelRgb(ref.r,ref.g,ref.b);
	}
      } else {
	PixelBgra ref = pbase->pixel(x,y);
	dest2(x,y) = PixelRgb(ref.r,ref.g,ref.b);
      }
    }
  }
  return true;
}

void PicmixEffect::readEffectData() {
  if (workDir==readDir&&workVersion==readVersion) {
    printf("Reading pixmix effect data from dir [%s] (version [%s])\n",
	   workDir.c_str(),
	   workVersion.c_str());
    return;
  }

  printf("Reading pixmix effect data from dir [%s] (version [%s])\n",
	 workDir.c_str(),
	 workVersion.c_str());
  readDir = workDir;
  readVersion = workVersion;

  string configFile = readDir;
  if (readDir!="") { configFile += "/"; }
  string base = configFile;
  configFile += "config.ini";
  printf("Reading config file %s\n", configFile.c_str());
  bool ok = effectConfig.fromConfigFile(configFile.c_str());
  if (!ok) {
    printf("Failed to read config file %s\n", configFile.c_str());
    configFile = base + "effect.ini";
    printf("Reading config file %s\n", configFile.c_str());
    ok = effectConfig.fromConfigFile(configFile.c_str());
    if (!ok) {
      printf("Failed to read config file %s\n", configFile.c_str());
      return;
    }
  }

  ConstString parent = effectConfig.check("parent",Value("")).asString();
  if (parent!="") {
    string cfg2 = (base+"/"+parent.c_str()+"/config.ini").c_str();
    printf("Reading options also from %s\n", cfg2.c_str());
    effectConfig.fromConfigFile(cfg2.c_str(),false);
    effectConfig.fromConfigFile(configFile.c_str(),false);
  }

  printf("Effect configuration is %s\n", effectConfig.toString().c_str());
  string icache = effectConfig.check("icache",Value("%04d.png")).asString().c_str();
  string xcache = effectConfig.check("xcache",Value("outx_%04d.png")).asString().c_str();
  string ycache = effectConfig.check("ycache",Value("outy_%04d.png")).asString().c_str();
  iseq.setPattern((base+icache).c_str());
  xseq.setPattern((base+xcache).c_str());
  yseq.setPattern((base+ycache).c_str());
  //printf("Reading from %s\n", buf);
  //img.load((base+frame).c_str());
  //printf("image size %dx%d\n", img.width(), img.height());
  first = effectConfig.check("first",Value(1)).asInt();
  last = effectConfig.check("last",Value(1)).asInt();
  first = effectConfig.check("firstAvailable",Value(first)).asInt();
  last = effectConfig.check("lastAvailable",Value(last)).asInt();
  step = effectConfig.check("step",Value(5)).asInt();
  linger = effectConfig.check("linger",Value(2)).asInt();
  current = first;
  currentLinger = 0;
}

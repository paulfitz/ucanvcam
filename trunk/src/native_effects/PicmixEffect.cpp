#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Effect.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

#include <string>

using namespace yarp::os;
using namespace yarp::sig;

using namespace std;


#include <ImageLoader.h>

#include "location.h"

/**
 *
 * An effect that uses a compiled still/animation as a template.
 *
 */
class PicmixEffect : public Effect {
public:
  PicmixEffect() {
    frame = -1;
    readDir = "";
    readVersion = "0";
    options.put("dir","picmix");
    options.put("version",0);
    needRead = false;
    reconfigure(options);
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
  int frame;
  string workDir, readDir;
  string workVersion, readVersion;
  Property options;
  Property effectConfig;
  ImageLoader img;
  bool needRead;
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

  if (img.isValid()) {
    for (int x=0; x<dest2.width()&&x<img.width(); x++) {
      for (int y=0; y<dest2.height()&&y<img.height(); y++) {
	PixelBgra v = img.pixel(x,y);
	if (v.a==0) {
	  dest2(x,y) = PixelRgb(v.r,v.g,v.b);
	} else if (v.a>=127) {
	  // do nothing, leave copied value
	} else {
	  PixelRgb& o = dest2(x,y);
	  float f = v.a/127.0;
	  if (f>1) f = 1;
	  int r = (int)((1-f)*v.r+f*o.r);
	  int g = (int)((1-f)*v.g+f*o.g);
	  int b = (int)((1-f)*v.b+f*o.b);
	  dest2(x,y) = PixelRgb(r,g,b);
	}
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
    return;
  }
  printf("Effect configuration is %s\n", effectConfig.toString().c_str());
  string frame = effectConfig.check("frame",Value("default.png")).asString().c_str();
  img.load((base+frame).c_str());
  printf("image size %dx%d\n", img.width(), img.height());
}

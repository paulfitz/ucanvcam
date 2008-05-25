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

/**
 *
 * An effect STUB that uses a compiled blender still/animation as a template.
 * It reads from a working directory that needs to be configured -
 * the default is to look for a "picmix" subdirectory.  Note, this is 
 * a STUB, it has not yet been implemented.
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
    reconfigure(options);
  }

  virtual bool draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2);

  virtual std::string getName() {
    return "PicmixTV";
  }

  virtual bool reconfigure(yarp::os::Searchable& config) {
    options.fromString(config.toString());
    if (options.check("dir")) {
      workDir = options.check("dir",Value("picmix")).asString().c_str();
    }
    if (options.check("version")) {
      workVersion = options.check("version",Value("")).toString().c_str();
    }
    readEffectData();
    return true;
  }

  virtual Property getConfiguration() {
    return options;
  }

  void readEffectData() {
    if (workDir!=readDir||workVersion!=readVersion) {
      printf("Reading pixmix effect data from dir [%s] (version [%s])\n",
	     readVersion.c_str(),
	     readDir.c_str());
      readDir = workDir;
      readVersion = workVersion;
    } else {
      printf("Pixmix effect data is current for dir [%s] (version [%s])\n",
	     readVersion.c_str(),
	     readDir.c_str());
    }
  }
private:
  int frame;
  string workDir, readDir;
  string workVersion, readVersion;
  Property options;
};

Effect *picmixRegister() {
  return new PicmixEffect();
}

bool PicmixEffect::draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2) {
  printf("*** warning: Picmix effect not implemented yet\n");
  printf("*** warning: if it were, it would look in %s\n", workDir.c_str());
  dest2 = src2;
  return true;
}


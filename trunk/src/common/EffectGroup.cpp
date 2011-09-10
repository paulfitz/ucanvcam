
extern "C" {
#include "EffecTV.h"
  //#include "utils.h"
#include "effect_list.h"
}

#include "EffectGroup.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

// -> not present
//// -> floating point exception
#ifdef USE_GD
static effectRegisterFunc *effects_register_list[] =
{
	dumbRegister,
	quarkRegister,
	fireRegister,
	burnRegister,
	blurzoomRegister,
	streakRegister,
	baltanRegister,
	onedRegister,
	//dotRegister,
	mosaicRegister,
	puzzleRegister,
	//predatorRegister,
	spiralRegister,
	simuraRegister,
	edgeRegister,
	shagadelicRegister,
	noiseRegister,
	agingRegister,
	TransFormRegister,
	lifeRegister,
	sparkRegister,
	warpRegister,
	//holoRegister,
	cycleRegister,
	rippleRegister,
	diceRegister,
	dizzyRegister,
	DeinterlaceRegister,
	nervousRegister,
	rndmRegister,
	revRegister,
	rdsRegister,
	lensRegister,
	diffRegister,
	scrollRegister,
	warholRegister,
	// DROP until register and init are separated -
	// image size can change
	//matrixRegister,
	//pupRegister,
	//chameleonRegister,
	//opRegister,
	//nervousHalfRegister,
	//slofastRegister,
	////displayWallRegister,
	//bluescreenRegister,
	//colstreakRegister,
	//timeDistortionRegister,
	//edgeBlurRegister,
};
#endif

static effect **effectsList;
static int effectMax;
static int currentEffectNum;
static effect *currentEffect = NULL;

#define RAW(x) ((RGB32*)(x).getRawImage())

static ImageOf<PixelBgra> srcTV, destTV;

class TestEffect : public Effect {
public:
  TestEffect() {
  }

  virtual bool draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2) {
	dest2 = src2;
	return true;
  }

  virtual std::string getName() {
    return "TestTV";
  }
};

Effect *testRegister() {
  return new TestEffect();
}


/**
 *
 * Wrap an effect from the EffecTV project.
 *
 */
class PortEffectTV : public Effect { //, public DriverCreator {
public:
  std::string name;
  effect *e;

  PortEffectTV() {
    name = "none";
    e = NULL;
  }

  PortEffectTV(const char *name, effect *e) {
    this->name = name;
    this->e = e;
  }

  void config(const char *name, effect *e) {
    this->name = name;
    this->e = e;
  }

  virtual ~PortEffectTV() {
    if (e!=NULL) free(e);
    e = NULL;
  }

  virtual yarp::sig::Image *pdraw(yarp::sig::Image& src,
				  yarp::sig::Image& dest) {
    srcTV.copy(src);
    destTV.resize(src);
    //dest.zero();
    screen->pixels = RAW(destTV);
    if (e->draw!=NULL) {
      e->draw(RAW(srcTV),RAW(destTV));
    }
    return &destTV;
  }

  std::string getName() {
    return name;
  }
  
  bool start() {
    if (e!=NULL) {
      int result = e->start();
      if (result>=0) return true;
    }
    return false;
  }

  bool stop() {
    if (e!=NULL) {
      int result = e->stop();
      if (result>=0) return true;
    }
    return false;
  }
};


void EffectGroup::add(Effect *effect) {
  effects.push_back(effect);
  //printf("%s Registered\n",effect->getName().c_str());
  effectMax++;
}

int EffectGroup::init()
{
	int i, n;
	effect *entry;

	effectMax = 0;
#ifdef USE_GD
	n = sizeof(effects_register_list)/sizeof(effectRegisterFunc *);
	for(i=0;i<n;i++) {
		entry = (*effects_register_list[i])();
		if(entry) {
		  //effectsList[effectMax] = entry;
		  add(new PortEffectTV(entry->name,entry));
		  //Drivers::factory().add(
		}
	}
#endif
	add(testRegister());
#ifdef USE_SDL
	add(tickerRegister());
	add(engageRegister());
	add(paramRegister());
	add(overlayRegister());
	add(picmixRegister());
#endif
	printf("%d effects are available.\n",effectMax);
	return effectMax;
}


static int changeEffect(int num)
{
/* return value:
 *  0: fatal error
 *  1: success
 *  2: not available
 */
	if(currentEffect)
		currentEffect->stop();
	currentEffectNum = num;
	while(currentEffectNum < 0)
		currentEffectNum += effectMax;
	while(currentEffectNum >= effectMax)
		currentEffectNum -= effectMax;
	currentEffect = effectsList[currentEffectNum];
	screen_setcaption(currentEffect->name);
	//screen_clear(0);
	//if(stretch) {
	  //		image_stretching_buffer_clear(0);
	//}
	if(currentEffect->start() < 0)
		return 2;

	return 1;
}

int searchEffect(const char *name)
{
	int i, num, len1, len2;
	char *p;

	len1 = strlen(name);
	if(len1 > 2) {
		if(strncasecmp(&name[len1-2], "TV", 2) == 0) {
			len1 -= 2;
		}
	}

	num = -1;
	for(i=0; i<effectMax; i++) {
		p = effectsList[i]->name;
		len2 = strlen(p);
		if(len2 > 2) {
			if(strncasecmp(&p[len2-2], "TV", 2) == 0) {
				len2 -= 2;
			}
		}
		if(len1 != len2)
			continue;
		if(strncasecmp(name, effectsList[i]->name, len1) == 0) {
			num = i;
			break;
		}
	}
	if(num == -1) {
		fprintf(stderr, "Couldn't find \"%s\". Starts DumbTV.\n", name);
		num = 0;
	}

	return num;
}


effect *getEffect(const char *name) {
  int num = searchEffect(name);
  if (num==-1) { return NULL; }
  if (changeEffect(num)==1) {
    return currentEffect;
  }
  return NULL;
}


Effect *searchGeneralEffect(const char *str) {
	return NULL;
}




///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

EffectGroup::EffectGroup() {
}


/*
int EffectGroup::init() {
  int i, n;
  effect *entry;
  
  n = sizeof(effects_register_list)/sizeof(effectRegisterFunc *);
  effectsList = (effect **)malloc(n*sizeof(effect *));
  effectMax = 0;
  for(i=0;i<n;i++) {
    entry = (*effects_register_list[i])();
    if(entry) {
      printf("%s OK.\n",entry->name);
      effectsList[effectMax] = entry;
      effectMax++;
    }
  }
  printf("%d effects are available.\n",effectMax);
  return effectMax;
}
*/

Effect *EffectGroup::search(const char *str) {
  for (int i=0; i<(int)effects.size(); i++) {
    if (effects[i]->getName() == str) {
      //printf("Active effect is %s\n", str);
      return effects[i];
    }
  }
  printf("Could not find effect %s\n", str);
  return NULL;
}


EffectGroup *g_effects = NULL;

EffectGroup& EffectGroup::get() {
  if (g_effects == NULL) {
    g_effects = new EffectGroup;
    g_effects->init();
  }
  return *g_effects;
}



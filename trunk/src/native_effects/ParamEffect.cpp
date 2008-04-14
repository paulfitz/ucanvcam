#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/all.h>

#include "EffectGroup.h"


using namespace yarp::os;
using namespace yarp::sig;

#ifndef WIN32
#define NETWORKED 1
#endif

class ParamEffect : public YarpEffect, public PortReader {
private:
  YarpEffect *current;
  BufferedPort<Bottle> msg;
  Semaphore mutex;
public:
  ParamEffect() : mutex(1) {
    current = NULL;
    msg.setReplier(*this);
  }

  void apply(Bottle& in,Bottle& out) {
    Bottle *bot = &in;
    out.clear();
    printf("Got message %s\n", bot->toString().c_str());
    ConstString cmd = bot->get(0).asString();
    if (cmd == "set") {
      ConstString str = bot->get(1).asString();
      YarpEffect *next = YarpEffects::get().search(str.c_str());
      if (next==NULL) {
	next = YarpEffects::get().search("BrokenTV");
      }
      if (next!=NULL) {
	if (current!=NULL) {
	  current->stop();
	}
	next->start();
	current = next;
      }
      out.fromString("[ok]");
    } else if (cmd == "list") {
      out = YarpEffects::get().getList();
    } else {
      out.fromString("[fail] \"command not recognized\"");
    }
  }

  virtual bool read(yarp::os::ConnectionReader& connection) {
    Bottle in, out;
    bool ok = in.read(connection);
    if (!ok) return false;

    apply(in,out);

    ConnectionWriter *returnToSender = connection.getWriter();
    if (returnToSender!=NULL) {
      out.write(*returnToSender);
    }
    return true;
  }

  virtual bool start() {
#ifdef NETWORKED
    bool ok = msg.open("/ctrl");
    if (!ok) return false;
#endif
    if (current==NULL) {
      current = YarpEffects::get().search("TickerTV");
      return current->start();
    }
    return false;
  }

  virtual bool stop() {
#ifdef NETWORKED
    msg.close();
#endif
    if (current!=NULL) {
      return current->stop();
    }
    return true;
  }

  virtual yarp::sig::Image *pdraw(yarp::sig::Image& src,
				  yarp::sig::Image& dest) {
    /*
#ifdef NETWORKED
    Bottle *bot = msg.read(0);
#else
    Bottle *bot = NULL;
#endif
    */
    yarp::sig::Image *result = NULL;
    mutex.wait();
    if (current!=NULL) {
      result = current->pdraw(src,dest);
    } else {
      dest.copy(src);
      result = &dest;
    }
    mutex.post();
    return result;
  }

  std::string getName() {
    return "ParamTV";
  }
};

YarpEffect *yparamRegister() {
  return new ParamEffect();
}


//yarp::sig::Image *ParamEffect::pdraw(yarp::sig::Image& src,
//				     yarp::sig::Image& dest) {
//}




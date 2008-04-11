
#include <yarp/os/Bottle.h>
#include <yarp/os/ConstString.h>

class ISourceLister {
public:
  virtual ~ISourceLister() {}

  virtual yarp::os::Bottle getSources() = 0;

  virtual yarp::os::ConstString guessSource() = 0;
};

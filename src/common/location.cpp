
#include "location.h"

#include "config.h"

#include "registry.h"
#include "registry_keys.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include <yarp/os/Time.h>

using namespace std;
using namespace yarp::os;

static string g_location = UCANVCAM_RESOURCE_PATH;
static bool g_located = false;

std::string getResourceLocation() {
  //printf("Checking for resource location\n");
  if (!g_located) {
    std::string result = getRegistry(KEY_NEW_ROOT);
    if (result!="") {
      //printf("Registry reports dll resource location %s\n", result.c_str());
      g_location = result + "/media";
    } else {
      result = getRegistry(KEY_ROOT);
      if (result!="") {
	//printf("Registry reports resource location %s\n", result.c_str());
	g_location = result;
      } else {
#ifndef WIN32
	if (access("media",R_OK)==0) {
	  char buf[1000];
	  // freefont may need complete paths
	  getcwd(buf,sizeof(buf));
	  g_location = buf;
	  g_location += "/media";
	} else if (access("../media",R_OK)==0) {
	  char buf[1000];
	  // freefont may need complete paths
	  getcwd(buf,sizeof(buf));
	  g_location = buf;
	  g_location += "/../media";
	} else if (access(UCANVCAM_RESOURCE_PATH,R_OK)==0) {
	  g_location = UCANVCAM_RESOURCE_PATH;
	} else {
	  Time::delay(1);
	  fprintf(stderr,"Cannot find media directory.\n");
	  fprintf(stderr,"Continuing, but images and fonts will be missing.\n");
	  Time::delay(2);
	}
#endif
      }
    }
    g_located = true;
  }
  return g_location;
}



void setResourceLocation(std::string location) {
  putRegistry(KEY_ROOT,location.c_str());
}


#include "location.h"

#include "config.h"

#include "registry.h"
#include "registry_keys.h"

using namespace std;

static string g_location = UCANVCAM_RESOURCE_PATH;
static bool g_located = false;

std::string getResourceLocation() {
  //printf("Checking for resource location\n");
  if (!g_located) {
    std::string result = getRegistry(KEY_ROOT);
    if (result!="") {
      printf("Registry reports resource location %s\n", result.c_str());
      g_location = result;
    }
  }
  return g_location;
}



void setResourceLocation(std::string location) {
  putRegistry(KEY_ROOT,location.c_str());
}

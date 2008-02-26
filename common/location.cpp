
#include "location.h"

#include "config.h"

#include "registry.h"
#include "registry_keys.h"

using namespace std;

static string g_location = SWEETCAM_RESOURCE_PATH;
static bool g_located = false;

std::string getResourceLocation() {
  if (!g_located) {
    std::string result = getRegistry(KEY_ROOT);
    if (result!="") {
      g_location = result;
    }
  }
  return g_location;
}




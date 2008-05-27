#include <yarp/sig/Image.h>
using namespace yarp::sig;

#include <assert.h>

#define MAPCACHE_SCALE (256*64.0)

/**
 *
 * Interpret cached picmix data.
 *
 */
class MapCache {
private:
  int local1, local2;
  PixelBgra& pix1;
  PixelBgra& pix2;
public:
  MapCache(PixelBgra& p1, PixelBgra& p2) : pix1(p1), pix2(p2) {
  }

  MapCache(int p1, int p2) : pix1(*((PixelBgra*)(&local1))),
			     pix2(*((PixelBgra*)(&local2))) {
    local1 = p1;
    local2 = p2;
  }

  void clear() {
    pix1 = PixelBgra(0,0,0,0);
    pix2 = PixelBgra(0,0,0,0);
  }

  int getX() {
    return pix1.r+pix1.g*256;
  }

  int getY() {
    return pix2.r+pix2.g*256;
  }

  int getIndex() {
    return pix1.b%16;
  }

  int getLumen() {
    return pix2.b;
  }

  int getDistance() {
    return pix1.a%16;
  }

  double getScale() {
    return MAPCACHE_SCALE;
  }
};


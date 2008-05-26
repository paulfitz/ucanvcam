#ifndef IMAGELOADER_INC
#define IMAGELOADER_INC

#include <yarp/sig/Image.h>

#include <gd.h>

/**
 * Very light wrapper around gd for reading images from file.
 */
class ImageLoader {
private:
  gdImagePtr im;
public:
  ImageLoader() { im = NULL; }

  virtual ~ImageLoader();

  bool load(const char *fname);

  void clear();
  
  yarp::sig::PixelBgra pixel(int x, int y) {
    int v = gdImageGetPixel(im,x,y);
    return *((yarp::sig::PixelBgra *)(&v));
  }

  bool isValid() {
    return im!=NULL;
  }

  int width() {
    if (im!=NULL) return im->sx;
    return 0;
  }

  int height() {
    if (im!=NULL) return im->sy;
    return 0;
  }
};

#endif

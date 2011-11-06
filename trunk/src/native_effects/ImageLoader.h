#ifndef IMAGELOADER_INC
#define IMAGELOADER_INC

#include <yarp/os/Property.h>
#include <yarp/sig/Image.h>

#ifdef GD_IS_AVAILABLE
#include <gd.h>
#else
typedef void *gdImagePtr;
#endif

#if _MSC_VER
#define snprintf _snprintf
#endif

#include <vector>
#include <string>

#include <stdio.h>

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
#if GD_IS_AVAILABLE
    int v = gdImageGetPixel(im,x,y);
    return *((yarp::sig::PixelBgra *)(&v));
#else
	return yarp::sig::PixelBgra();
#endif
  }

  bool isValid() {
    return im!=NULL;
  }

  int width() {
#ifdef GD_IS_AVAILABLE
    if (im!=NULL) return im->sx;
#endif
    return 0;
  }

  int height() {
#ifdef GD_IS_AVAILABLE
    if (im!=NULL) return im->sy;
#endif
    return 0;
  }

  gdImagePtr take() {
    gdImagePtr result = im;
    im = NULL;
    return result;
  }
};


/**
 * Cache a sequence of images on demand.
 */
class ImageSeq {
private:
  std::vector <ImageLoader *> store;
  yarp::os::Property idx;
  int at;
  int ext;
  std::string extKey;
  std::string pattern;
public:
  ImageSeq() {
    at = -1;
    ext = -1;
    pattern = "";
  }

  virtual ~ImageSeq() {
    clear();
  }

  void clear() {
    at = -1;
    ext = -1;
    pattern = "";
    for (int i=0; i<(int)store.size(); i++) {
      if (store[i]!=NULL) {
	delete store[i];
	store[i] = NULL;
      }
    }
    idx.clear();
  }

  void setPattern(const char *pattern) {
    clear();
    this->pattern = pattern;
  }

  bool go(int index) {
    ext = index;
    char buf[1000];
    snprintf(buf,sizeof(buf),"%d",index);
    extKey = buf;
    yarp::os::Value v = idx.find(extKey.c_str());
    //printf("Checked %s -> %s\n", extKey.c_str(), v.toString().c_str());
    if (!idx.check(extKey.c_str())) {
      if (pattern=="") {
	at = -1;
	return false;
      } else {
	char buf2[1000];
	snprintf(buf2,sizeof(buf2),pattern.c_str(),index);
	ImageLoader *img = new ImageLoader;
	if (img==NULL) {
	  printf("Out of memory\n");
	  at = -1;
	  return false;
	}
	bool ok = img->load(buf2);
	if (!ok) {
	  printf("(Could not load %s)\n", buf2);
	  //delete img;
	  //at = -1;
	  //return false;
	}
	return add(img);
      }
    }
    at = v.asInt();
    return true;
  }

  bool add(ImageLoader *loader) {
    at = store.size();
    idx.put(extKey.c_str(),at);
    printf("Adding %s -> %d\n", extKey.c_str(), at);
    store.push_back(loader);
    return true;
  }

  ImageLoader *get() {
    if (at!=-1) {
      return store[at];
    }
    return NULL;
  }
};

#endif

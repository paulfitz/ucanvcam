#define __forceinline inline

#include <vd2/Riza/capdrivers.h>
#include <vd2/Riza/capdriver.h>

#include <vd2/Riza/bitmap.h>

#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

using namespace nsVDCapture;

//extern HWND ghApp;


//#define xprintf(a) printf(a); if (FOUT!=NULL) {fprintf(FOUT,a); fflush(FOUT);}
//#define xprintf2(a1,a2) printf(a1,a2); if (FOUT!=NULL) {fprintf(FOUT,a1,a2); fflush(FOUT);}

#define xprintf(a) printf(a);
#define xprintf2(a1,a2) printf(a1,a2);



class Publisher {
public:
    virtual bool accept(yarp::sig::ImageOf<yarp::sig::PixelBgr>& img) = 0;
};


class VDINTERFACE MyCallback : public IVDCaptureDriverCallback {
public:
    Publisher *pub;

    MyCallback() { pub = NULL; }

    void setPublisher(Publisher *pub) {
        this->pub = pub;
    }

  vdstructex<BITMAPINFOHEADER> format;

  virtual void CapBegin(sint64 global_clock) {
    xprintf("Capture begins\n");
  }

  virtual void CapEnd(const MyError *pError) {
    xprintf("Capture ends\n");
  }

  virtual bool CapEvent(nsVDCapture::DriverEvent event, int data) {
    xprintf("Capture event\n");
    
	switch(event) {
	case kEventPreroll:
	  xprintf("  event preroll\n");
	  break;

	case kEventCapturing:
	  xprintf("  capturing\n");
	  break;

	case kEventVideoFrameRateChanged:
	case kEventVideoFormatChanged:
	  xprintf("  video format\n");
	  break;

	case kEventVideoFramesDropped:
	  xprintf("  dropped\n");
	  break;

	case kEventVideoFramesInserted:
	  xprintf("  inserted\n");
	  break;

	case kEventVideoSourceChanged:
	  xprintf("  video changed\n");
	  break;

	case kEventAudioSourceChanged:
	  xprintf("  audio changed\n");
	  break;
	}

	return true;

  }

  virtual void CapProcessData(int stream, const void *data, uint32 size, sint64 timestamp, bool key, sint64 global_clock) {
    xprintf("Capture data\n");
    xprintf2("Stream is %d\n", stream);
    xprintf2("format width %d\n", format->biWidth);
    xprintf2("format height %d\n", format->biHeight);
    xprintf2("format size %ld\n", format->biSize);
    xprintf2("incoming size %ld\n", size);
    if (size==format->biWidth*format->biHeight*3) {
      xprintf("understandable!\n");
      char buf[256];
      static int ct = 0;
      sprintf(buf,"img_%06d.ppm",ct);
      ct++;
      yarp::sig::ImageOf<yarp::sig::PixelBgr> img;
      img.setQuantum(1);
      img.setTopIsLowIndex(false);
      img.setExternal((char*)data,format->biWidth,format->biHeight);
      //write(img,buf);
      if (pub!=NULL) {
          pub->accept(img);
      }
    }
  }

};


class WinCap : public yarp::dev::DeviceDriver, 
	       public yarp::dev::IFrameGrabberImage,
	       public Publisher
{
private:
    bool closed;
    int ww, hh;
    yarp::sig::ImageOf<yarp::sig::PixelRgb> *target;
    yarp::os::Semaphore ready, read;
    MyCallback callback;
    IVDCaptureSystem *cap;
    IVDCaptureDriver *dd;
  HWND ghApp;

public:
    WinCap() : ready(0), read(0) {
        ww = hh = -1;
        closed = true;
        callback.setPublisher(this);
        dd = NULL;
        cap = NULL;
	ghApp = 0;
    }

    virtual int width() const {
        return ww;
    }

    int height() const {
        return hh;
    }
  
    virtual bool open(yarp::os::Searchable& config);

    virtual bool close() {
        if (!closed) {
            read.post();
            if (dd!=NULL) {
                dd->CaptureStop();
            }
            xprintf("Capture stop\n");
            if (dd!=NULL) {
                delete dd;
                dd = NULL;
            }
            if (cap!=NULL) {
                delete cap;
                cap = NULL;
            }
        }
        closed = true;
        //ready.post();
        return true;
    }

    bool accept(yarp::sig::ImageOf<yarp::sig::PixelBgr>& image) {
        if (closed) return false;
        xprintf("Got some camera data!\n");
        ready.wait();
        if (target!=NULL) {
            target->copy(image);
        }
        read.post();
        return true;
    }

    // yarp::dev::IFrameGrabberImage
    virtual bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>& image) {
        xprintf("Reading some camera data...\n");
        if (closed) return false;
        bool ok = false;
        target = &image;
        ready.post();
        read.wait();
        if (closed) return false;
        xprintf("Read some camera data...\n");
        return true;
    }
};

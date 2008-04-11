#define _WIN32_IE 0x400
#include <windows.h>
#include <commctrl.h>

#pragma warning(disable:4244)
#pragma warning(disable:4711)

//class IBaseFilter;
//typedef void *LPDDPIXELFORMAT;

#include <stdlib.h>

#include <windows.h>

#include <streams.h>
#include <amfilter.h>
#include "source.h"
#include <stdio.h>
#include <olectl.h>
#include <dvdmedia.h>
#include <ddraw.h>
#include "Filters.h"

#include "yarpy.h"
//using namespace yarp::sig;

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>
//#include "drivers.h"

//#include "Register.h"

using namespace yarp;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;
using namespace yarp::dev;

HWND ghApp=0;
FILE *FOUT = NULL;

//#include "WinCap.h"

// WINE
//#include <wine/uuidof.h>
//#define _uuidof uuidof

// REPLACED _uuidof with IID_*

//#define NETWORKED

//////////////////////////////////////////////////////////////////////////
//  CVCam is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////
CUnknown * WINAPI CVCam::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
  printf("Hello, making a ucanvcam virtual camera\n");
  //Logger::get().silence();
  Network::init();
#ifdef NETWORKED
#else
  Time::turboBoost();
#endif
  //yarp::dev::DriverCollection dev;
    //Drivers::factory().add(new DriverCreatorOf<WinCap>("wincap",
    //                                                 "grabber",
    //                                                 ""));
    ASSERT(phr);
    CUnknown *punk = new CVCam(lpunk, phr);
    return punk;
}

CVCam::CVCam(LPUNKNOWN lpunk, HRESULT *phr) : 
    CSource(NAME("ucanvcam virtual camera"), lpunk, CLSID_VirtualCam)
{
    ASSERT(phr);
    CAutoLock cAutoLock(&m_cStateLock);
    // Create the one and only output pin
    m_paStreams = (CSourceStream **) new CVCamStream*[1];
    m_paStreams[0] = new CVCamStream(phr, this, L"Virtual Cam");
}

HRESULT CVCam::QueryInterface(REFIID riid, void **ppv)
{
  printf("Query interface CVCam\n");
    //Forward request for IAMStreamConfig & IKsPropertySet to the pin
  //if(riid == _uuidof(IAMStreamConfig) || riid == _uuidof(IKsPropertySet))
  if(riid == IID_IAMStreamConfig || riid == IID_IKsPropertySet ||
     riid == IID_ISpecifyPropertyPages || riid == IID_ISaturation ||
     riid == IID_IAMVfwCaptureDialogs)
    return m_paStreams[0]->QueryInterface(riid, ppv);
  else
    return CSource::QueryInterface(riid, ppv);
}


STDMETHODIMP CVCam::NonDelegatingQueryInterface(REFIID riid, void **ppv) 
{
  printf("non-delegating Query interface CVCam\n");
  if(riid == IID_IAMStreamConfig || riid == IID_IKsPropertySet ||
     riid == IID_ISpecifyPropertyPages || riid == IID_ISaturation)
    return m_paStreams[0]->NonDelegatingQueryInterface(riid, ppv);
  else
    return CSource::NonDelegatingQueryInterface(riid, ppv);
}


#ifdef NETWORKED

#define SAY(x) { Bottle& b = outPort.prepare(); b.clear(); b.add(Value(x)); printf(">>> %s\n", b.toString().c_str()); outPort.write();  }

#define SAY3(x,y,z) { Bottle& b = outPort.prepare(); b.clear(); b.add(Value(x)); b.add(Value(y)); b.add(Value(z)); printf(">>> %s\n", b.toString().c_str()); outPort.write();  }

#else

#define SAY(x) { Bottle b; b.clear(); b.add(Value(x)); printf(">>> %s\n", b.toString().c_str());   }

#define SAY3(x,y,z) { Bottle b; b.clear(); b.add(Value(x)); b.add(Value(y)); b.add(Value(z)); printf(">>> %s\n", b.toString().c_str());   }

#endif

//////////////////////////////////////////////////////////////////////////
// CVCamStream is the one and only output pin of CVCam which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////
CVCamStream::CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName) :
    CSourceStream(NAME("ucanvcam virtual camera"),phr, pParent, pPinName), m_pParent(pParent)
{
  printf("Starting up CVCamStream\n"); fflush(stdout);
  CAutoLock cAutoLockShared(&m_cSharedState);
  bus.init();
  ct = 0;
    // Set the default media type as 320x240x24@15
    GetMediaType(4, &m_mt);
    printf("  *** %s:%d\n", __FILE__, __LINE__); fflush(stdout);
#ifdef NETWORKED
    Network::setLocalMode(true);
    printf("  *** %s:%d\n", __FILE__, __LINE__); fflush(stdout);
    Contact c = Contact::byName("/bozo").addSocket("tcp","127.0.0.1",9999);
      //Network::registerContact(c);
    printf("  *** %s:%d\n", __FILE__, __LINE__); fflush(stdout);
    outPort.open(c);
#endif
    printf("  *** %s:%d\n", __FILE__, __LINE__); fflush(stdout);
    Property pSource;
    //pSource.put("device","vfw_grabber");
    pSource.put("device","vdub");
    //pSource.put("device","test_grabber");
    pSource.put("mode","ball");
    pSource.put("width",320);
    pSource.put("height",240);
    pSource.put("w",320);
    pSource.put("h",240);
    printf("  *** %s:%d\n", __FILE__, __LINE__); fflush(stdout);
    bool ok = imgSrc.open(pSource);
    ASSERT(ok);
    printf("  *** %s:%d\n", __FILE__, __LINE__); fflush(stdout);
    running = true;
    printf("Started up CVCamStream\n"); fflush(stdout);
}

CVCamStream::~CVCamStream()
{
  SAY("~CVCamStream");
  CAutoLock cAutoLockShared(&m_cSharedState);
  imgSrc.close();
#ifdef NETWORKED
  outPort.close();
#endif
  bus.fini();
  //Network::fini();
} 

HRESULT CVCamStream::QueryInterface(REFIID riid, void **ppv)
{   
  printf("Query interface CVCamStream\n");
  SAY("QueryInterface");

    if(riid == IID_IAMStreamConfig)
      printf("IAMStreamConfig\n");
    else if(riid == IID_IKsPropertySet)
      printf("IKsPropertySet\n");
    else if(riid == IID_ISpecifyPropertyPages)
      printf("ISpecifyPropertyPages\n");
    else if(riid == IID_ISaturation)
      printf("ISaturation\n");
    else if(riid == IID_IAMVfwCaptureDialogs) 
      printf("vfwcapture\n");
    else
      printf("God knows\n");
    fflush(stdout);

    // Standard OLE stuff
    if(riid == IID_IAMStreamConfig)
        *ppv = (IAMStreamConfig*)this;
    else if(riid == IID_IKsPropertySet)
        *ppv = (IKsPropertySet*)this;
    else if(riid == IID_ISpecifyPropertyPages)
      *ppv = (ISpecifyPropertyPages*)this;
    else if(riid == IID_ISaturation)
      *ppv = (ISaturation*)this;
    else if(riid == IID_IAMVfwCaptureDialogs) 
      *ppv = (IAMVfwCaptureDialogs*)this;
    else
        return CSourceStream::QueryInterface(riid, ppv);

    AddRef();
    return S_OK;
}



STDMETHODIMP CVCamStream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
  printf("non-delegating Query interface CVCamStream\n");

    if(riid == IID_IAMStreamConfig)
      printf("IAMStreamConfig\n");
    else if(riid == IID_IKsPropertySet)
      printf("IKsPropertySet\n");
    else if(riid == IID_ISpecifyPropertyPages)
      printf("ISpecifyPropertyPages\n");
    else if(riid == IID_ISaturation)
      printf("ISaturation\n");
    else
      printf("God knows\n");
    fflush(stdout);


    if(riid == IID_IAMStreamConfig)
        *ppv = (IAMStreamConfig*)this;
    else if(riid == IID_IKsPropertySet)
        *ppv = (IKsPropertySet*)this;
    else if (riid == IID_ISpecifyPropertyPages)
        *ppv = (ISpecifyPropertyPages*)this;
    else if (riid == IID_ISaturation)
        *ppv = (ISaturation*)this;
    else
      // Call the parent class.
      return CSourceStream::NonDelegatingQueryInterface(riid, ppv);


    AddRef();
    return S_OK;
}



//////////////////////////////////////////////////////////////////////////
//  This is the routine where we create the data being output by the Virtual
//  Camera device.
//////////////////////////////////////////////////////////////////////////

HRESULT CVCamStream::FillBuffer(IMediaSample *pms)
{
  SAY3("FillBuffer",0,(int)m_lSaturation);

  CheckPointer(pms,E_POINTER);
  HRESULT hr;

  {
    CAutoLock cAutoLockShared(&m_cSharedState);

    //if (!running) {
    //return NOERROR;
    //}

    //ImageOf<PixelRgb> img;
  //img.resize(320,240);

  IFrameGrabberImage *grabber = NULL;
  imgSrc.view(grabber);
  ImageOf<PixelRgb> img;
  if (grabber!=NULL) {
    //SAY("Getting image");
    grabber->getImage(img);
    //SAY3("Got image",img.width(),img.height());
  } else {
    SAY("Image diversion");
    Time::delay(0.03); 

    bus.beginRead();
    ImageOf<PixelRgb> cp;
    cp.setQuantum(1);
    cp.setExternal(bus.buffer(),320,240);
    img.copy(cp);
    bus.endRead();
  }
#ifdef MERGE_SERVICE
  getServerImage(img);
#endif

  //return NOERROR;

  BYTE *pData;
  long lDataLen;
  if (FAILED(hr=pms->GetPointer(&pData))||!pData) 
    return S_FALSE;

  lDataLen = pms->GetSize();

  ASSERT(m_mt.formattype == FORMAT_VideoInfo);



    REFERENCE_TIME rtNow;
    
    REFERENCE_TIME avgFrameTime = ((VIDEOINFOHEADER*)m_mt.pbFormat)->AvgTimePerFrame;

    rtNow = m_rtLastTime;
    m_rtLastTime += avgFrameTime;
    pms->SetTime(&rtNow, &m_rtLastTime);

    //for(int i = 0; i < lDataLen; ++i)
    //pData[i] = rand();

    int ww = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biWidth;
    int hh = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biHeight;
    int bb = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biBitCount;

    if (img.width()==0) {
      // BGR order, apparently
      int pp = bb/8;
      int stride = (ww * pp + 3) & ~3;
      ct = (ct+1)%256;
      for (int y=0; y<hh; y++) {
	BYTE *base = pData + y*stride;
	for (int x=0; x<ww; x++) {
	  base[0] = 255;  //red?
	  base[1] = ct;
	  base[2] = 0;
	  base += pp;
	}
      }
    } else {
      //ImageOf<PixelRgb> dest1;
      ImageOf<PixelBgr> dest2;
      //dest1.setTopIsLowIndex(false);
      dest2.setTopIsLowIndex(false);
      dest2.setExternal((char*)pData,ww,hh);
      //dest1.resize(ww,hh);
      if (m_lSaturation<50) {
	//yarpy_apply(img,dest2);
	dest2.copy(img);
      } else {
	dest2.copy(img);
      }
    }
  }
    /*
    // and if you believe that, how about this...
    ImageOf<PixelRgb> src, dest;
    // quantum rules seem similar, check this though
    src.setExternal((char*)pData,ww,hh);
    dest.setExternal((char*)pData,ww,hh);
    //yarpy_apply(src,dest);
    */

    pms->SetSyncPoint(TRUE);

    return NOERROR;
} // FillBuffer


//
// Notify
// Ignore quality management messages sent from the downstream filter
STDMETHODIMP CVCamStream::Notify(IBaseFilter * pSender, Quality q)
{
  SAY("Notify");
    CheckPointer(pSender,E_POINTER);
    return E_NOTIMPL;
} // Notify

//////////////////////////////////////////////////////////////////////////
// This is called when the output format has been negotiated
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::SetMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->Format());
    HRESULT hr = CSourceStream::SetMediaType(pmt);
    return hr;
}

// See Directshow help topic for IAMStreamConfig for details on this method
HRESULT CVCamStream::GetMediaType(int iPosition, CMediaType *pmt)
{
  SAY("MediaType");
    CheckPointer(pmt,E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 8) return VFW_S_NO_MORE_ITEMS;

    if(iPosition == 0) 
    {
        *pmt = m_mt;
        return S_OK;
    }

    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));

    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biBitCount    = 24;
    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth      = 80 * iPosition;
    pvi->bmiHeader.biHeight     = 60 * iPosition;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    pvi->AvgTimePerFrame = 1000000;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);
    
    return NOERROR;

} // GetMediaType

// This method is called to see if a given output format is supported
HRESULT CVCamStream::CheckMediaType(const CMediaType *pMediaType)
{
  SAY("CheckMediaType");
    CheckPointer(pMediaType,E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)(pMediaType->Format());
    if(*pMediaType != m_mt) 
        return E_INVALIDARG;
    return S_OK;
} // CheckMediaType

// This method is called after the pins are connected to allocate buffers to stream data
HRESULT CVCamStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
  SAY("DecideBufferSize");
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = NOERROR;

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);

    if(FAILED(hr)) return hr;
    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;

    return NOERROR;
} // DecideBufferSize

// Called when graph is run
HRESULT CVCamStream::OnThreadCreate()
{
  SAY("OnThreadCreate");
    CAutoLock cAutoLockShared(&m_cSharedState);
    m_rtLastTime = 0;
    return NOERROR;
} // OnThreadCreate

HRESULT CVCamStream::OnThreadDestroy()
{
  //SAY("OnThreadDestroy-ing");
    CAutoLock cAutoLockShared(&m_cSharedState);
    //running = false;
    //SAY("OnThreadDestroy-ed");
    m_rtLastTime = 0;
    return NOERROR;
} // OnThreadDestroy


//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CVCamStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
  SAY("SetFormat");
    CheckPointer(pmt,E_POINTER);
    DECLARE_PTR(VIDEOINFOHEADER, pvi, m_mt.pbFormat);
    m_mt = *pmt;
    IPin* pin; 
    ConnectedTo(&pin);
    if(pin)
    {
        IFilterGraph *pGraph = m_pParent->GetGraph();
        pGraph->Reconnect(this);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
  SAY("GetFormat");
    CheckPointer(ppmt,E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    *ppmt = CreateMediaType(&m_mt);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
  SAY("GetNumberOfCapabilities");
    CheckPointer(piCount,E_POINTER);
    CheckPointer(piSize,E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    *piCount = 8;
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{
  SAY("GetStreamCaps");
    CheckPointer(pmt,E_POINTER);
    CheckPointer(pSCC,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    *pmt = CreateMediaType(&m_mt);
    DECLARE_PTR(VIDEOINFOHEADER, pvi, (*pmt)->pbFormat);

    if (iIndex == 0) iIndex = 4;

    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biBitCount    = 24;
    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth      = 80 * iIndex;
    pvi->bmiHeader.biHeight     = 60 * iIndex;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    (*pmt)->majortype = MEDIATYPE_Video;
    (*pmt)->subtype = MEDIASUBTYPE_RGB24;
    (*pmt)->formattype = FORMAT_VideoInfo;
    (*pmt)->bTemporalCompression = FALSE;
    (*pmt)->bFixedSizeSamples= FALSE;
    (*pmt)->lSampleSize = pvi->bmiHeader.biSizeImage;
    (*pmt)->cbFormat = sizeof(VIDEOINFOHEADER);
    
    DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
    
    pvscc->guid = FORMAT_VideoInfo;
    pvscc->VideoStandard = AnalogVideo_None;
    pvscc->InputSize.cx = 640;
    pvscc->InputSize.cy = 480;
    pvscc->MinCroppingSize.cx = 80;
    pvscc->MinCroppingSize.cy = 60;
    pvscc->MaxCroppingSize.cx = 640;
    pvscc->MaxCroppingSize.cy = 480;
    pvscc->CropGranularityX = 80;
    pvscc->CropGranularityY = 60;
    pvscc->CropAlignX = 0;
    pvscc->CropAlignY = 0;

    pvscc->MinOutputSize.cx = 80;
    pvscc->MinOutputSize.cy = 60;
    pvscc->MaxOutputSize.cx = 640;
    pvscc->MaxOutputSize.cy = 480;
    pvscc->OutputGranularityX = 0;
    pvscc->OutputGranularityY = 0;
    pvscc->StretchTapsX = 0;
    pvscc->StretchTapsY = 0;
    pvscc->ShrinkTapsX = 0;
    pvscc->ShrinkTapsY = 0;
    pvscc->MinFrameInterval = 200000;   //50 fps
    pvscc->MaxFrameInterval = 50000000; // 0.2 fps
    pvscc->MinBitsPerSecond = (80 * 60 * 3 * 8) / 5;
    pvscc->MaxBitsPerSecond = 640 * 480 * 3 * 8 * 50;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////


HRESULT CVCamStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
                        DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{// Set: Cannot set any properties.
  SAY("Set");
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CVCamStream::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
)
{
  SAY("Get");
    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;
    
    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK; // Caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;// The buffer is too small.
        
    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CVCamStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
  SAY("QuerySupported");
    if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
    // We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CVCamStream::HasDialog(/* [in] */ int iDialog) {
  if (iDialog==VfwCaptureDialog_Display) {
    return S_OK;
  }
  return S_FALSE;
}
        
HRESULT STDMETHODCALLTYPE CVCamStream::ShowDialog(/* [in] */ int iDialog,
						  /* [in] */ HWND hwnd) {

  /*
  HWND hdlg;

  hdlg = CreateDialogParam(g_hInst,
			   MAKEINTRESOURCE(IDS_PROPPAGE),
			   hWnd, //GetDesktopWindow(),
			   pDlgProc,
			   lParam);
  */  

  CGrayProp propPage((ISaturation*)this);
  PROPPAGEINFO pageInfo;
  propPage.GetPageInfo(&pageInfo);
  RECT rect;
  rect.left = 10;
  rect.top = 10;
  rect.right = rect.left+320;
  rect.bottom = rect.top+240;
  propPage.Activate(hwnd,&rect,false);

  return S_OK;
}
  
HRESULT STDMETHODCALLTYPE CVCamStream::SendDriverMessage(/* [in] */int iDialog,
							 /* [in] */ int uMsg,
							 /* [in] */ long dw1,
							 /* [in] */ long dw2) {
  return S_OK;
    //E_INVALIDARG;
}











HRESULT CGrayProp::OnConnect(IUnknown *pUnk)
{
    if (pUnk == NULL)
    {
        return E_POINTER;
    }
    ASSERT(m_pGray == NULL);
    return pUnk->QueryInterface(IID_ISaturation, 
        reinterpret_cast<void**>(&m_pGray));
}


HRESULT CGrayProp::OnActivate(void)
{
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_BAR_CLASSES;
    if (InitCommonControlsEx(&icc) == FALSE)
    {
        return E_FAIL;
    }

    ASSERT(m_pGray != NULL);
    HRESULT hr = m_pGray->GetSaturation(&m_lVal);
    if (SUCCEEDED(hr))
    {
        SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETRANGE, 0,
            MAKELONG(0, 100));

        SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETTICFREQ, 
            (100 - 0) / 10, 0);

        SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETPOS, 1, m_lVal);


        SendDlgItemMessage(m_Dlg, IDC_EFFECTS, CB_ADDSTRING, 
			   0,(LPARAM)"hello");
        SendDlgItemMessage(m_Dlg, IDC_EFFECTS, CB_ADDSTRING, 
			   0,(LPARAM)"there");

    }
    return hr;
}


BOOL CGrayProp::OnReceiveMessage(HWND hwnd,
    UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_DEFAULT)
        {
            // User clicked the 'Revert to Default' button.
            m_lNewVal = 50;
            m_pGray->SetSaturation(m_lNewVal);

            // Update the slider control.
            SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETPOS, 1,
                m_lNewVal);
            SetDirty();
            return (LRESULT) 1;
        }
        break;

    case WM_HSCROLL:
        {
            // User moved the slider.
            switch(LOWORD(wParam))
            {
            case TB_PAGEDOWN:
            case SB_THUMBTRACK:
            case TB_PAGEUP:
                m_lNewVal = SendDlgItemMessage(m_Dlg, IDC_SLIDER1,
                    TBM_GETPOS, 0, 0);
                m_pGray->SetSaturation(m_lNewVal);
                SetDirty();
            }
            return (LRESULT) 1;
        }
    } // Switch.
    
    // Let the parent class handle the message.
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
} 


HRESULT CGrayProp::OnDisconnect(void)
{
    if (m_pGray)
    {
        // If the user clicked OK, m_lVal holds the new value.
        // Otherwise, if the user clicked Cancel, m_lVal is the old value.
        m_pGray->SetSaturation(m_lVal);  
        m_pGray->Release();
        m_pGray = NULL;
    }
    return S_OK;
}


CUnknown * WINAPI CGrayProp::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
{
  printf(">>>>>>>>> making a property object\n"); fflush(stdout);
    CGrayProp *pNewObject = new CGrayProp(pUnk);
    if (pNewObject == NULL) 
    {
        *pHr = E_OUTOFMEMORY;
    }
    return pNewObject;
} 



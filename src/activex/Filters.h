// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2008 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

// Based on an example by rep movsd via the mad hatter

#ifndef UCANVCAM_FILTERS
#define UCANVCAM_FILTERS

//#include <yarp/os/BufferedPort.h>
//#include <yarp/os/Bottle.h>
//#include <yarp/dev/PolyDriver.h>

#include "resources.h"
#include "ShmemBus.h"

#include "YarpOut.h"

EXTERN_C const GUID CLSID_VirtualCam;

DEFINE_GUID(IID_ISaturation, 0x1195d708, 0x2f98, 
            0x4643, 0xa7, 0xfe, 0x59, 0x47, 0x04, 0xe4, 0xfc, 0xb4);

interface ISaturation : public IUnknown
{
    STDMETHOD(GetSaturation)(long *plSat) = 0;
    STDMETHOD(SetSaturation)(long lSat) = 0;
};

DEFINE_GUID(CLSID_SaturationProp, 0x1195d708, 0x2f98, 
            0x4643, 0xa7, 0xfe, 0x59, 0x47, 0x04, 0xe4, 0xfc, 0xb5);


class CVCamStream;
class CVCam : public CSource
{
public:

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    //STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

    //STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    IFilterGraph *GetGraph() {return m_pGraph;}


    virtual ~CVCam();

private:
    CVCam(LPUNKNOWN lpunk, HRESULT *phr);
    YarpOut yarp_out;
};

class CVCamStream : public CSourceStream
/*, public IAMStreamConfig, 
                    public IKsPropertySet,
                    public ISaturation,
                    public ISpecifyPropertyPages,
                    public IAMVfwCaptureDialogs */
{
public:

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    //STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }                                                          \
    STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

    //STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    


    //////////////////////////////////////////////////////////////////////////
    //  IQualityControl
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    //////////////////////////////////////////////////////////////////////////
    //  IAMStreamConfig
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

    //////////////////////////////////////////////////////////////////////////
    //  IKsPropertySet
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData,DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
    
    //////////////////////////////////////////////////////////////////////////
    //  CSourceStream
    //////////////////////////////////////////////////////////////////////////
    CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName);
    ~CVCamStream();

    HRESULT FillBuffer(IMediaSample *pms);
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT OnThreadCreate(void);
    HRESULT OnThreadDestroy(void);

    STDMETHODIMP GetSaturation(long *plSat)
    {
        if (!plSat) return E_POINTER;
        CAutoLock lock(&m_cSharedState);
        *plSat = m_lSaturation;
        return S_OK;
    }
    STDMETHODIMP SetSaturation(long lSat)
    {
        CAutoLock lock(&m_cSharedState);
        if (lSat < 0 || lSat > 100)
            {
                return E_INVALIDARG;
            }
        m_lSaturation = lSat;
        return S_OK;
    }

    STDMETHODIMP GetPages(CAUUID *pPages)
    {
        CAutoLock cAutoLock(m_pFilter->pStateLock());
        printf("******** Getting pages\n");
        if (pPages == NULL) return E_POINTER;
        pPages->cElems = 1;
        pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
        if (pPages->pElems == NULL) 
            {
                return E_OUTOFMEMORY;
            }
        pPages->pElems[0] = CLSID_SaturationProp;
        printf("******** Returned page\n");
        return S_OK;
    }


    virtual HRESULT STDMETHODCALLTYPE HasDialog(/* [in] */ int iDialog);
        
    virtual HRESULT STDMETHODCALLTYPE ShowDialog(/* [in] */ int iDialog,
                                                 /* [in] */ HWND hwnd);
  
    virtual HRESULT STDMETHODCALLTYPE SendDriverMessage(/* [in] */ int iDialog,
                                                        /* [in] */ int uMsg,
                                                        /* [in] */ long dw1,
                                                        /* [in] */ long dw2);


private:
    CVCam *m_pParent;
    REFERENCE_TIME m_rtLastTime;
    HBITMAP m_hLogoBmp;
    CCritSec m_cSharedState;
    IReferenceClock *m_pClock;
    long      m_lSaturation; // Saturation level.
    bool running;
    int ct;
    ShmemBus bus;
    double lastChange;
    int lastId;
    bool haveId;
    YarpOut yarp_out;
};


class CGrayProp : public CBasePropertyPage
{
private:
    ISaturation *m_pGray;    // Pointer to the filter's custom interface.
    long        m_lVal;       // Store the old value, so we can revert.
    long        m_lNewVal;   // New value.

    void SetDirty()
    {
        m_bDirty = TRUE;
        if (m_pPageSite)
            {
                m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
            }
    }

public:
    CGrayProp(IUnknown *pUnk) : 
        CBasePropertyPage(NAME("GrayProp"), pUnk, IDD_PROPPAGE, 
                          IDS_PROPPAGE_TITLE),
        m_pGray(0)
    { 
        yarp_out.init("gray");
    }

    /* ... */


    HRESULT OnConnect(IUnknown *pUnk);

    HRESULT OnActivate(void);

    BOOL OnReceiveMessage(HWND hwnd,
                          UINT uMsg, WPARAM wParam, LPARAM lParam);


    HRESULT OnApplyChanges(void)
    {
        m_lVal = m_lNewVal;
        return S_OK;
    } 

    HRESULT OnDisconnect(void);


    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);

    YarpOut yarp_out;

};

#endif

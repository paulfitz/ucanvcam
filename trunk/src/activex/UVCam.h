#ifndef UCANVCAM_FILTERS_INC
#define UCANVCAM_FILTERS_INC

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

#include "ShmemBus.h"

#ifndef UV_CODE
#define UV_CODE 10
#endif

#define PASTER(x,y) x ## _ ## y
#define EVALUATOR(x,y)  PASTER(x,y)
#define LSTRINGIFY(s) L ## #s
#define LXSTRINGIFY(s) LSTRINGIFY(s)
#define STRINGIFY(s) #s
#define XSTRINGIFY(s) STRINGIFY(s)
#define LQUOTED_CAM_NAME LXSTRINGIFY(EVALUATOR(UCANVCAM,UV_CODE))
#define QUOTED_CAM_NAME STRINGIFY(EVALUATOR(UCANVCAM,UV_CODE))

EXTERN_C const GUID CLSID_VirtualCam;

class UVCamStream;
class UVCam : public CSource
{
public:
	//////////////////////////////////////////////////////////////////////////
	//  IUnknown
	//////////////////////////////////////////////////////////////////////////
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

	IFilterGraph *GetGraph() {return m_pGraph;}

private:
	UVCam(LPUNKNOWN lpunk, HRESULT *phr);

	int GetPinCount();
	CBasePin *GetPin(int n);
};

class UVCamStream : public CSourceStream, public IAMStreamConfig, public IKsPropertySet
{
public:

	//////////////////////////////////////////////////////////////////////////
	//  IUnknown
	//////////////////////////////////////////////////////////////////////////
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }                                                          \
		STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

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
	UVCamStream(HRESULT *phr, UVCam *pParent, LPCWSTR pPinName);
	~UVCamStream();

	HRESULT FillBuffer(IMediaSample *pms);
	HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);
	HRESULT OnThreadCreate(void);


private:
	UVCam *m_pParent;
	REFERENCE_TIME m_rtLastTime;
	int m_iRepeatTime;
	const int m_iDefaultRepeatTime;
	CRefTime m_rtSampleTime;

	HBITMAP m_hLogoBmp;
	CCritSec m_cSharedState;
	IReferenceClock *m_pClock;
	ShmemBus bus;

	double lastChange;
	double firstTime;
	int lastId;
	bool haveId;
	int ct;
};


#endif


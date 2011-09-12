#pragma warning(disable:4244)
#pragma warning(disable:4711)

#include <streams.h>
#include <stdio.h>
#include <olectl.h>
#include <dvdmedia.h>

#include "UVCam.h"

// For MinGW
#ifndef _uuidof
#define _uuidof(x) IID_ ## x
#endif

#include "ShmemImageRaw.h"

#include "DllDbg.h"

//////////////////////////////////////////////////////////////////////////
//  UVCam is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////
CUnknown * WINAPI UVCam::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	ASSERT(phr);
	CUnknown *punk = new UVCam(lpunk, phr);
	DllDbg out; out.init("uvcam"); out.say("uvcam begins");
	return punk;
}

UVCam::UVCam(LPUNKNOWN lpunk, HRESULT *phr) : 
CSource(NAME(QUOTED_CAM_NAME), lpunk, CLSID_VirtualCam) {
	ASSERT(phr);
	CAutoLock cAutoLock(&m_cStateLock);
	// Create the one and only output pin
	m_paStreams = (CSourceStream **) new UVCamStream*[1];
	m_paStreams[0] = new UVCamStream(phr, this, LQUOTED_CAM_NAME);
}

HRESULT UVCam::QueryInterface(REFIID riid, void **ppv)
{
	//Forward request for IAMStreamConfig & IKsPropertySet to the pin
	if(riid == _uuidof(IAMStreamConfig) || riid == _uuidof(IKsPropertySet))
		return m_paStreams[0]->QueryInterface(riid, ppv);
	else
		return CSource::QueryInterface(riid, ppv);
}

int UVCam::GetPinCount() { return 1; }

CBasePin *UVCam::GetPin(int n) {
	CAutoLock cAutoLock(&m_cStateLock);
	return m_paStreams[0];
}

//////////////////////////////////////////////////////////////////////////
// UVCamStream is the one and only output pin of UVCam which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////
UVCamStream::UVCamStream(HRESULT *phr, UVCam *pParent, LPCWSTR pPinName) :
CSourceStream(NAME(QUOTED_CAM_NAME),phr, pParent, pPinName), m_pParent(pParent), m_iDefaultRepeatTime(50)
{
	bus.init();
	lastId = 0;
	lastChange = -1000;
	haveId = false;
	ct = 0;
	// Set the default media type as 320x240x24@15
	GetMediaType(4, &m_mt);
}

UVCamStream::~UVCamStream()
{
	bus.fini();
} 

HRESULT UVCamStream::QueryInterface(REFIID riid, void **ppv)
{   
	// Standard OLE stuff
	if(riid == _uuidof(IAMStreamConfig))
		*ppv = (IAMStreamConfig*)this;
	else if(riid == _uuidof(IKsPropertySet))
		*ppv = (IKsPropertySet*)this;
	else
		return CSourceStream::QueryInterface(riid, ppv);

	AddRef();
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//  This is the routine where we create the data being output by the Virtual
//  Camera device.
//////////////////////////////////////////////////////////////////////////

HRESULT UVCamStream::FillBuffer(IMediaSample *pms) {

	CheckPointer(pms,E_POINTER);
	HRESULT hr;

	{
		CAutoLock cAutoLockShared(&m_cSharedState);

		int wt = 0;
		double now = 0;
		bool ok = false;
		char *at = NULL;
		int at_width = 0;
		int at_height = 0;

		while (wt<10 && !ok) {
			wt++;
			bus.beginRead();
			ShmemImageHeader header;
			ShmemImageRaw cp(bus);
			cp.getHeader(header);
			at = cp.getImage();

			FILETIME tm;
			GetSystemTimeAsFileTime(&tm);

			ULONGLONG ticks = (((ULONGLONG) tm.dwHighDateTime) << 32) + 
				tm.dwLowDateTime;
			now = ((double)ticks)/(1000.0*1000.0*10.0);
			if (firstTime<0) {
				firstTime = now;
			}
			now -= firstTime;

			at_width = header.get(IMGHDR_W);
			at_height = header.get(IMGHDR_H);
			int id = header.get(IMGHDR_TICK);
			if (!haveId) {
				lastId = id;
				haveId = true;
			} else {
				if (id!=lastId) {
					lastChange = now;
					wt = 500;
					ok = true;
				}
			}
			lastId = id;

			if (!ok) {
				bus.endRead();
				Sleep(5);
			}
		}

		if (ok) {
			ok = (now-lastChange<2);
		}

		BYTE *pData = NULL;
		long lDataLen;
		if (FAILED(hr=pms->GetPointer(&pData)))  {
			bus.endRead();
			return S_FALSE;
		}

		lDataLen = pms->GetSize();

		AM_MEDIA_TYPE *nmt = NULL;
		if (pms->GetMediaType(&nmt)==S_OK) {
			m_mt = *nmt;
		}

		ASSERT(m_mt.formattype == FORMAT_VideoInfo);

		/*
		REFERENCE_TIME rtNow;
		REFERENCE_TIME avgFrameTime = ((VIDEOINFOHEADER*)m_mt.pbFormat)->AvgTimePerFrame;
		rtNow = m_rtLastTime;
		m_rtLastTime += avgFrameTime;
		pms->SetTime(&rtNow, &m_rtLastTime);
		*/

		int ww = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biWidth;
		int hh = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biHeight;
		int bb = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biBitCount;

		DllDbg out; out.init("uvcam",true); out.say("FillBuffer output dimensions", ww, hh);
		out.say("FillBuffer input dimensions", at_width, at_height);
		out.say("FillBuffer sequence, time", (int)at, (int)now);

		// compatibility with current ucanvcam binaries
		if (at_width==0&&at_height==0) {
			at_width = 320;  at_height = 240;
		}

		int pp = bb/8;
		int stride = (ww * pp + 3) & ~3;
		int gap = (at_width-ww)*3;
		if (gap<0) gap = 0;
		if (at==NULL || !ok) {
			// BGR order, apparently
			ct = (ct+1)%256;
			for (int y=0; y<hh; y++) {
				BYTE *base = pData + y*stride;
				for (int x=0; x<ww; x++) {
					base[0] = 255; //(y+ct)%256;
					base[1] = ((y+x+ct)%20==0)?128:0; //ct;
					base[2] = 0; //(x+ct)%256;
					base += pp;
				}
			}
		} else {
			for (int y=(hh<at_width)?hh:at_height-1; y>=0; y--) {
				BYTE *base = pData + y*stride;
				for (int x=0; x<ww && x<at_width; x++) {
					base[2] = *at; at++;
					base[1] = *at; at++;
					base[0] = *at; at++;
					base += pp;
				}
				at += gap;
			}
		}
		bus.endRead();

		REFERENCE_TIME avgFrameTime = ((VIDEOINFOHEADER*)m_mt.pbFormat)->AvgTimePerFrame;
		CRefTime rnow;
		m_pParent->StreamTime(rnow);
		REFERENCE_TIME endThisFrame = rnow + avgFrameTime;
		pms->SetTime((REFERENCE_TIME *) &rnow, &endThisFrame);

	}

	pms->SetSyncPoint(TRUE);

	return NOERROR;
} // FillBuffer


//
// Notify
STDMETHODIMP UVCamStream::Notify(IBaseFilter * pSender, Quality q) {
	if(q.Proportion<=0)
	{
		m_iRepeatTime = 1000;
	}
	else
	{
		m_iRepeatTime = m_iRepeatTime*1000 / q.Proportion;
		if(m_iRepeatTime>1000)
		{
			m_iRepeatTime = 1000;
		}
		else if(m_iRepeatTime<10)
		{
			m_iRepeatTime = 10;
		}
	}

	if(q.Late > 0)
		m_rtSampleTime += q.Late;

	return NOERROR;

}

//////////////////////////////////////////////////////////////////////////
// This is called when the output format has been negotiated
//////////////////////////////////////////////////////////////////////////
HRESULT UVCamStream::SetMediaType(const CMediaType *pmt)
{
	CheckPointer(pmt,E_POINTER); 
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->Format());
	HRESULT hr = CSourceStream::SetMediaType(pmt);
	return hr;
}

// See Directshow help topic for IAMStreamConfig for details on this method
HRESULT UVCamStream::GetMediaType(int iPosition, CMediaType *pmt)
{
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
HRESULT UVCamStream::CheckMediaType(const CMediaType *pMediaType)
{
	CheckPointer(pMediaType,E_POINTER); 
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)(pMediaType->Format());
	if(*pMediaType != m_mt) 
		return E_INVALIDARG;
	return S_OK;
} // CheckMediaType

// This method is called after the pins are connected to allocate buffers to stream data
HRESULT UVCamStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
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
HRESULT UVCamStream::OnThreadCreate()
{
	m_rtLastTime = 0;
	m_rtSampleTime = 0;
	m_iRepeatTime = m_iDefaultRepeatTime;
	firstTime = -1;

	return NOERROR;
} // OnThreadCreate


//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE UVCamStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
	if(!pmt) return E_POINTER;
	CAutoLock cAutoLock(m_pFilter->pStateLock());
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

HRESULT STDMETHODCALLTYPE UVCamStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
	CheckPointer(ppmt,E_POINTER); 
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	*ppmt = CreateMediaType(&m_mt);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE UVCamStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	CheckPointer(piCount,E_POINTER); 
	CheckPointer(piSize,E_POINTER); 
	*piCount = 8;
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE UVCamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{
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


HRESULT UVCamStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
	DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{// Set: Cannot set any properties.
	return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT UVCamStream::Get(
	REFGUID guidPropSet,   // Which property set.
	DWORD dwPropID,        // Which property in that set.
	void *pInstanceData,   // Instance data (ignore).
	DWORD cbInstanceData,  // Size of the instance data (ignore).
	void *pPropData,       // Buffer to receive the property data.
	DWORD cbPropData,      // Size of the buffer.
	DWORD *pcbReturned     // Return the size of the property.
	)
{
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
HRESULT UVCamStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
	if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
	// We support getting this property, but not setting it.
	if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
	return S_OK;
}



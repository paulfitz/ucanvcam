// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2007 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

#ifndef VFW_EXTRA_PAULFITZ
#define VFW_EXTRA_PAULFITZ

#ifndef capSetCallbackOnFrame
#define capSetCallbackOnFrame(handle, callback) (safeSendMessage (handle, WM_CAP_SET_CALLBACK_FRAME, 0, callback))
#endif

#ifndef capSetCallbackOnStatus
#define capSetCallbackOnStatus(handle, callback) (safeSendMessage (handle, WM_CAP_SET_CALLBACK_STATUS, 0, callback))
#endif

#ifndef capSetCallbackOnCapControl
#define capSetCallbackOnCapControl(handle, callback) (safeSendMessage (handle, WM_CAP_SET_CALLBACK_CAPCONTROL, 0, callback))
#endif

#ifndef capSetCallbackOnVideoStream
#define capSetCallbackOnVideoStream(handle, callback) (safeSendMessage (handle, WM_CAP_SET_CALLBACK_VIDEOSTREAM, 0, callback))
#endif

#ifndef capSetCallbackOnWaveStream
#define capSetCallbackOnWaveStream(handle, callback) (safeSendMessage (handle, WM_CAP_SET_CALLBACK_WAVESTREAM, 0, callback))
#endif

#ifndef capGetAudioFormatSize
#define capGetAudioFormatSize(handle) (safeSendMessage (handle, WM_CAP_GET_AUDIOFORMAT, 0, 0))
#endif

#ifndef capGrabFrameNoStop
#define capGrabFrameNoStop(handle) (safeSendMessage (handle, WM_CAP_GRAB_FRAME_NOSTOP, 0, 0))
#endif

#ifndef capGetAudioFormat
#define capGetAudioFormat(handle,data,len) (safeSendMessage (handle, WM_CAP_GET_AUDIOFORMAT, len, data))
#endif

#ifndef capSetAudioFormat
#define capSetAudioFormat(handle,data,len) (safeSendMessage (handle, WM_CAP_SET_AUDIOFORMAT, len, data))
#endif

#ifndef capGetStatus
#define capGetStatus(handle,data,len) (safeSendMessage (handle, WM_CAP_GET_STATUS, len, data))
#endif


#ifndef capPreviewScale
#define capPreviewScale(handle, flag) (safeSendMessage (handle, WM_CAP_SET_SCALE, flag, 0))
#endif

#ifndef IDS_CAP_BEGIN
#define IDS_CAP_BEGIN 300
#define IDS_CAP_END 301

#define CONTROLCALLBACK_PREROLL 1
#define CONTROLCALLBACK_CAPTURING 2

#endif


#endif



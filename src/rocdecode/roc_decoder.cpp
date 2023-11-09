/*
Copyright (c) 2023 - 2023 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "../commons.h"
#include "roc_decoder.h"

RocDecoder::RocDecoder(RocDecoderCreateInfo& decoder_create_info): va_video_decoder_{decoder_create_info}, device_id_{decoder_create_info.deviceid} {}

 RocDecoder::~RocDecoder() {
    // todo::
    hipError_t hipStatus = hipSuccess;
    if (hip_stream_) {
        hipStatus = hipStreamDestroy(hip_stream_);
        if (hipStatus != hipSuccess) {
            ERR("ERROR: hipStreamDestroy failed! (" + TOSTR(hipStatus) + ")");
        }
    }
 }

 rocDecStatus RocDecoder::InitializeDecoder() {
    rocDecStatus rocdec_status = ROCDEC_SUCCESS;
    rocdec_status = InitHIP(device_id_);
    if (rocdec_status != ROCDEC_SUCCESS) {
        ERR("ERROR: Failed to initilize the HIP!" + TOSTR(rocdec_status));
        return rocdec_status;
    }

    rocdec_status = va_video_decoder_.InitializeDecoder(hip_dev_prop_.gcnArchName);
    if (rocdec_status != ROCDEC_SUCCESS) {
        ERR("ERROR: Failed to initilize the VAAPI Video decoder!" + TOSTR(rocdec_status));
        return rocdec_status;
    }

     return rocdec_status;
 }

rocDecStatus RocDecoder::decodeFrame(RocdecPicParams *pPicParams) {
    rocDecStatus rocdec_status = ROCDEC_SUCCESS;
    rocdec_status = va_video_decoder_.SubmitDecode(pPicParams);
    if (rocdec_status != ROCDEC_SUCCESS) {
        ERR("ERROR: Decode submission is not successful!" + TOSTR(rocdec_status));
    }

     return rocdec_status;
}

rocDecStatus RocDecoder::getDecodeStatus(int nPicIdx, RocdecDecodeStatus* pDecodeStatus) {
    rocDecStatus rocdec_status = ROCDEC_SUCCESS;
    rocdec_status = va_video_decoder_.GetDecodeStatus(nPicIdx, pDecodeStatus);
    if (rocdec_status != ROCDEC_SUCCESS) {
        ERR("ERROR: Failed to query the decode status!" + TOSTR(rocdec_status));
    }
    return rocdec_status;
}

rocDecStatus RocDecoder::reconfigureDecoder(RocdecReconfigureDecoderInfo *pDecReconfigParams) {
    // todo:: return appropriate decStatus
    // this will be called when the current configuration is changed during decoding
    // release the current va-api decoder instance and create a new one with the new parameters (or reinit if available)
    // return status
    return ROCDEC_NOT_IMPLEMENTED;
}

rocDecStatus RocDecoder::mapVideoFrame(int nPicIdx, void *pDevMemPtr[3],
                                unsigned int pHorizontalPitch[3], RocdecProcParams *pVidPostprocParams) {
    // todo:: return appropriate decStatus
    // Post-process and map video frame corresponding to nPicIdx for use in HIP. Returns HIP device pointer and associated
    // pitch(horizontal stride) of the video frame. Returns device memory pointers for each plane (Y, U and V) seperately
    return ROCDEC_NOT_IMPLEMENTED;
}

rocDecStatus RocDecoder::unMapVideoFrame(void *pMappedDevPtr) {
    // todo:: return appropriate decStatus
    // Unmap a previously mapped video frame with the associated mapped raw pointer (pMappedDevPtr)
    return ROCDEC_NOT_IMPLEMENTED;
}


rocDecStatus RocDecoder::InitHIP(int device_id) {
    hipError_t hipStatus = hipSuccess;
    hipStatus = hipGetDeviceCount(&num_devices_);
    rocDecStatus decStatus = ROCDEC_SUCCESS;
    if (hipStatus != hipSuccess) {
        ERR("ERROR: hipGetDeviceCount failed!" + TOSTR(hipStatus));
        decStatus = ROCDEC_DEVICE_INVALID;
    }
    if (num_devices_ < 1) {
        ERR("ERROR: didn't find any GPU!");
        decStatus = ROCDEC_DEVICE_INVALID;
    }
    if (device_id >= num_devices_) {
        ERR("ERROR: the requested device_id is not found! ");
        decStatus = ROCDEC_DEVICE_INVALID;
    }
    hipStatus = hipSetDevice(device_id);
    if (hipStatus != hipSuccess) {
        ERR("ERROR: hipSetDevice( " + TOSTR(device_id) + ") failed! (" + TOSTR(hipStatus) + ")" );
        decStatus = ROCDEC_DEVICE_INVALID;
    }

    hipStatus = hipGetDeviceProperties(&hip_dev_prop_, device_id);
    if (hipStatus != hipSuccess) {
        ERR("ERROR: hipGetDeviceProperties for device (" +TOSTR(device_id) + " ) failed! (" + TOSTR(hipStatus) + ")" );
        decStatus = ROCDEC_DEVICE_INVALID;
    }

    hipStatus = hipStreamCreate(&hip_stream_);
    if (hipStatus != hipSuccess) {
        ERR("ERROR: hipStream_Create failed! (" + TOSTR(hipStatus) + ")");
        decStatus = ROCDEC_DEVICE_INVALID;
    }
    return decStatus;
}

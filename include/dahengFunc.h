#ifndef __DAHENG_FUNC_H__
#define __DAHENG_FUNC_H__

#include "GxIAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <memory>
#include <chrono>
#include <ctime>

#include <iostream>
#include <string>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/version.hpp>
#include <opencv2/highgui/highgui_c.h>

#define MEMORY_ALLOT_ERROR -1 


namespace daheng
{
    struct DahengCamera
    {
        
        bool g_get_image;                   ///< Flags that capture whether the thread ends: true runs; false exits
        GX_DEV_HANDLE g_device;              ///< Left camera Device handle
        GX_FRAME_DATA g_frame_data;         ///< Capture Image Parameters
        GX_OPEN_PARAM open_param;

        

        void InitializeCameraParameters(char* cameraIndex);
        GX_STATUS openCamera();

        //Get an error message description
        void GetErrorString(GX_STATUS error_status);

        //Acquisition thread functions
        GX_STATUS getImage(const  std::string imgName);

        //Get image size and request image data space
        GX_STATUS PreForImage();

        //Freeing resources
        GX_STATUS UnPreForImage();
        void SaveRawFile(void *image_buffer, size_t width, size_t height, std::string imgName);

        GX_STATUS closeCamera();

    };
   
    struct DahengDevice
    {
        DahengCamera stereoCamera;
       
        uint32_t device_num = 0;
        GX_FRAME_DATA stereoImgData;

        void InitializeBinocularParameters();
        GX_STATUS dahengDeviceInitLib();
        GX_STATUS getCameraList();
        GX_STATUS dahengDeviceCloseLib();
        GX_STATUS openBinocular();
        GX_STATUS acquisitionBinocularImages();

        GX_STATUS releaseImagesBuffer();
        GX_STATUS closeBinocular();

    };
 
}

#endif // DEBUG

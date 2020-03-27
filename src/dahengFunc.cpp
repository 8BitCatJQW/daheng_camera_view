#include"../include/dahengFunc.h"


using namespace cv;
using namespace std;

namespace daheng
{
    //-------------------------------DahengCamera------------------------------------------------------//
    void DahengCamera::GetErrorString(GX_STATUS error_status)
    {
        char *error_info = NULL;
        size_t size = 0;
        GX_STATUS status = GX_STATUS_SUCCESS;

        // Gets the length of the error description information
        status = GXGetLastError(&error_status, NULL, &size);
        if (status != GX_STATUS_SUCCESS)
        {
            GetErrorString(status);
            return;
        }

        error_info = new char[size];
        if (error_info == NULL)
        {
            printf("<Failed to allocate memory>\n");
            return;
        }

        // Get an error message description
        status = GXGetLastError(&error_status, error_info, &size);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXGetLastError call fail" << endl;
        }
        else
        {
            printf("%s\n", (char*)error_info);
        }

        // Freeing resources
        if (error_info != NULL)
        {
            delete[]error_info;
            error_info = NULL;
        }
    }

    GX_STATUS DahengCamera::getImage(const std::string imgName)
    {
       GX_STATUS status = GX_STATUS_SUCCESS;

       //Send Mining commands
       status = GXSendCommand(g_device, GX_COMMAND_TRIGGER_SOFTWARE);

       if (g_frame_data.pImgBuf == NULL)
       {
           return GX_STATUS_ERROR;
       }

       status = GXGetImage(g_device, &g_frame_data, 100);
      
       if (status != GX_STATUS_SUCCESS )
       {
           return GX_STATUS_ERROR;
       }


       return GX_STATUS_SUCCESS;

    }

    void DahengCamera::SaveRawFile(void *image_buffer, size_t width, size_t height, std::string imgName)
    {
        static int raw_file_index = 1;
        FILE* ff = NULL;
        string prefixStr = "../image/";
        prefixStr += imgName;
        ff = fopen(prefixStr.c_str(), "wb");
        cout << prefixStr << endl;
        if (ff != NULL)
        {
            fprintf(ff, "P5\n" "%u %u 255\n", width, height);
            fwrite(image_buffer, 1, width * height, ff);
            fclose(ff);
            ff = NULL;
        }
    }

    //-------------------------------------------------
    /**

    \brief Get image size and request image data space

    \return void

    */

    //-------------------------------------------------
    GX_STATUS DahengCamera::PreForImage()
    {
        GX_STATUS status = GX_STATUS_SUCCESS;
        
        int64_t payload_size = 0;

        status = GXGetInt(g_device, GX_INT_PAYLOAD_SIZE, &payload_size);
        

        if (status != GX_STATUS_SUCCESS )
        {
            GetErrorString(status);
            return status;
        }
        g_frame_data.pImgBuf = malloc(payload_size);
        
        if (g_frame_data.pImgBuf == NULL)
        {
            cout << "<Failed to allot memory" << endl;
            return MEMORY_ALLOT_ERROR;
        }
        return GX_STATUS_SUCCESS;
    }

    //-------------------------------------------------
    /**
    \brief Freeing resources
    \return void
    */
    //-------------------------------------------------

    GX_STATUS DahengCamera::UnPreForImage()
    {
        GX_STATUS status = GX_STATUS_SUCCESS;
       
        uint32_t ret = 0;
        //Send a stop-mining command
        status = GXSendCommand(g_device, GX_COMMAND_ACQUISITION_STOP);
       

        if (status != GX_STATUS_SUCCESS)
        {
            GetErrorString(status);
            return status;
        }
        g_get_image = false;

        if (ret != 0)
        {
            printf("<Failed to release resources>\n");
            return ret;
        }
        //Relaease buffer
        if (g_frame_data.pImgBuf != NULL)
        {
            free(g_frame_data.pImgBuf);
            g_frame_data.pImgBuf = NULL;
        }

        return GX_STATUS_SUCCESS;
    }

    void DahengCamera::InitializeCameraParameters(char* cameraIndex)
    {
        open_param.accessMode = GX_ACCESS_EXCLUSIVE;
        open_param.openMode = GX_OPEN_INDEX;
        open_param.pszContent = cameraIndex;
       
    }

    GX_STATUS DahengCamera::openCamera()
    {
        GX_STATUS status;
        status = GXOpenDevice(&open_param, &g_device);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXOpenDevice Failed!!!" << endl;
            return GX_STATUS_ERROR;
        }

        //Set up acquisition mode for continuous acquisition
        status = GXSetEnum(g_device, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXSetEnum GX_ACQ_MODE_CONTINUOUS Failed!!!" << endl;
            return GX_STATUS_ERROR;
        }

        //Set the trigger switch to ON
        status = GXSetEnum(g_device, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXSetEnum GX_TRIGGER_MODE_ON Failed!!!" << endl;
            return GX_STATUS_ERROR;
        }

        //Set the software trigger 
        status = GXSetEnum(g_device, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_SOFTWARE);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXSetEnum GX_TRIGGER_SOURCE_SOFTWARE Failed!!!" << endl;
            return GX_STATUS_ERROR;
        }

        //Preparing for collection    
        int ret = PreForImage();

        if (ret != 0)
        {
            printf("<Failed to prepare for acquire image>\n");
            status = GXCloseDevice(g_device);
            if (g_device != NULL)
            {
                g_device = NULL;
            }
            status = GXCloseLib();
            return GX_STATUS_ERROR;
        }

        ////Turn off Auto exposure
        //status = GXSetEnum(g_device, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
        //if (status != GX_STATUS_SUCCESS)
        //{
        //    cout << "GXSetEnum GX_EXPOSURE_AUTO_OFF Failed!!!" << endl;
        //    return GX_STATUS_ERROR;
        //}

        status = GXSetFloat(g_device, GX_FLOAT_EXPOSURE_TIME, 10000);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXSetEnum GX_FLOAT_EXPOSURE_TIME Failed" << endl;
            return GX_STATUS_ERROR;
        }

        status = GXSendCommand(g_device, GX_COMMAND_ACQUISITION_START);
        if (status != GX_STATUS_SUCCESS)
        {
            cout << "GXSendCommand  Failed!!!" << endl;
            return GX_STATUS_ERROR;
        }
        
        return GX_STATUS_SUCCESS;

    }

    GX_STATUS DahengCamera::closeCamera()
    {
        return GXCloseDevice(g_device);
    }

    GX_STATUS DahengDevice::dahengDeviceInitLib()
    {
        return GXInitLib();
    }
//-------------------------------------------DahengDevice ending------------------------------------------------------------//


//-----------------------------------DahengDevice------------------------------------------------------------------------//

    GX_STATUS DahengDevice::getCameraList()
    {
        return GXUpdateDeviceList(&device_num, 1000);
    }

    GX_STATUS DahengDevice::dahengDeviceCloseLib()
    {
        return GXCloseLib();
    }

    void DahengDevice::InitializeBinocularParameters()
    {
        stereoCamera.InitializeCameraParameters("1");
       
        cout << "Stereo Camera Setup Complete!!!" << endl;
    }

    GX_STATUS DahengDevice::openBinocular()
    {
        GX_STATUS stereoStatus = stereoCamera.openCamera();
       

        if (stereoStatus == GX_STATUS_ERROR )
        {
           
            cout << "Failed to turn on stereo camera " << endl;
            stereoCamera.closeCamera();
            dahengDeviceCloseLib();
            return GX_STATUS_ERROR;

        }
        cout << "Stereo Open successfully!!!" << endl;
        return GX_STATUS_SUCCESS;
    }

    GX_STATUS DahengDevice::acquisitionBinocularImages()
    {
        GX_STATUS lFlag = stereoCamera.getImage("left");
       

        if (lFlag == GX_STATUS_ERROR )
        {
           
           cout << "Failed acquisition stereo image " << endl;
          
           
            return GX_STATUS_ERROR;

        }
         
        this->stereoImgData = stereoCamera.g_frame_data;
       
        return GX_STATUS_SUCCESS;
    }

    GX_STATUS DahengDevice::releaseImagesBuffer()
    {
        GX_STATUS lFlag = stereoCamera.UnPreForImage();
       

        if (lFlag == GX_STATUS_ERROR )
        {
          
            cout << "Failed release stereo image buffer " << endl;
            return GX_STATUS_ERROR;

        }

        cout << "release stereo image buffer successfully!!!" << endl;
        return GX_STATUS_SUCCESS;


    }

    GX_STATUS DahengDevice::closeBinocular()
    {
        GX_STATUS lFlag = stereoCamera.closeCamera();
       
        if (lFlag == GX_STATUS_ERROR)
        {
           
            cout << "Failed close stereo camera " << endl;
            return GX_STATUS_ERROR;
        }

        cout << "Close stereo successfully!!!" << endl;
        return GX_STATUS_SUCCESS;
    }
//----------------------------------------DahengDevice ending---------------------------------------------------------------------------//
   
}



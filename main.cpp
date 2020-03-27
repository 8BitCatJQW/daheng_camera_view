#include "include/GxIAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <thread>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>

#include "include/dahengFunc.h"









using namespace std;
using namespace cv;
using namespace daheng;




float cropFactor = 0.5;

cv::Mat lmapx, lmapy, rmapx, rmapy;
DahengDevice dahengDevice;



bool sendFlag = false;








int main(int argc, char* argv[])
{
	
	
    GX_STATUS status;
  
    dahengDevice.dahengDeviceInitLib();

    dahengDevice.getCameraList();
    cout <<"Number of cameras found: " <<dahengDevice.device_num << endl;
    if (dahengDevice.device_num != 1)
    {
        cout << "No  cameras found.!!!" << endl;
        dahengDevice.dahengDeviceCloseLib();
        return -1;

    }

    dahengDevice.InitializeBinocularParameters();
    status = dahengDevice.openBinocular();
    if (status == GX_STATUS_ERROR)
    {
        return -1;
    }

    Mat leftR, rightR;
    time_t begin, end, end1,end2;
   //--------------------------------------------------------------------------
 
    //-----------------------SGM Initialization-------------------------------
    status = dahengDevice.acquisitionBinocularImages();
    if (status == GX_STATUS_ERROR)
    {
        cout << "Failed to get image" << endl;
        return 0;
    }
    int disp_size = 128;

    Mat stereoImg(cv::Size(dahengDevice.stereoImgData.nWidth, dahengDevice.stereoImgData.nHeight), CV_8UC1, (void*)dahengDevice.stereoImgData.pImgBuf, cv::Mat::AUTO_STEP);
    
    int widthS = dahengDevice.stereoImgData.nWidth;
  
  
   
 
     while (1)
    {
        dahengDevice.acquisitionBinocularImages();
        
        Mat stereoImg(cv::Size(dahengDevice.stereoImgData.nWidth, dahengDevice.stereoImgData.nHeight), CV_8UC1, (void*)dahengDevice.stereoImgData.pImgBuf, cv::Mat::AUTO_STEP);
        int widthS = dahengDevice.stereoImgData.nWidth;
        int heightS = dahengDevice.stereoImgData.nHeight;
        Rect rectL(0, 0, widthS * cropFactor, heightS);
        Rect rectR(widthS * cropFactor, 0, widthS * cropFactor, heightS);
        
        Mat Il(stereoImg, rectR);
        Mat Ir(stereoImg, rectL);
 	cv::imshow("test",Il);
	cv::waitKey(1);
       
    }
  

	

 


	

    dahengDevice.releaseImagesBuffer();
    dahengDevice.closeBinocular();
    dahengDevice.dahengDeviceCloseLib();


    return 0;
}


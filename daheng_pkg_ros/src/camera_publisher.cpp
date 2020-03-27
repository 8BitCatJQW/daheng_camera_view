#include "./dahengsrc/GxIAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <thread>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>

#include "./dahengsrc/dahengFunc.h"

#include <ros/ros.h>  
#include <image_transport/image_transport.h>  
#include <opencv2/highgui/highgui.hpp>  
#include <cv_bridge/cv_bridge.h> 



using namespace std;
using namespace cv;
using namespace daheng;




float cropFactor = 0.5;

cv::Mat lmapx, lmapy, rmapx, rmapy;
DahengDevice dahengDevice;



bool sendFlag = false;




int main(int argc, char* argv[])
{
   //ros node init
    ros::init(argc, argv, "image_publisher");  
    ros::NodeHandle nh;  
    image_transport::ImageTransport it(nh);  
    image_transport::Publisher pub = it.advertise("camera/image", 5);  
   
	
    //daheng camera initialize
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

   
 
    
    status = dahengDevice.acquisitionBinocularImages();
    if (status == GX_STATUS_ERROR)
    {
        cout << "Failed to get image" << endl;
        return 0;
    }
   


    sensor_msgs::ImagePtr msg;  

    ros::Rate loop_rate(5); 
     while (1)
    {
        dahengDevice.acquisitionBinocularImages();
        
        Mat stereoImg(cv::Size(dahengDevice.stereoImgData.nWidth, dahengDevice.stereoImgData.nHeight), CV_8UC1, (void*)dahengDevice.stereoImgData.pImgBuf, cv::Mat::AUTO_STEP);
	  if(!stereoImg.empty()) {  
          msg = cv_bridge::CvImage(std_msgs::Header(), "mono8", stereoImg).toImageMsg();  
          pub.publish(msg);  
          //cv::Wait(1);  
        }  

        ros::spinOnce();  
        loop_rate.sleep(); 
/* 
        int widthS = dahengDevice.stereoImgData.nWidth;
        int heightS = dahengDevice.stereoImgData.nHeight;
        Rect rectL(0, 0, widthS * cropFactor, heightS);
        Rect rectR(widthS * cropFactor, 0, widthS * cropFactor, heightS);
        
        Mat Il(stereoImg, rectR);
        Mat Ir(stereoImg, rectL);
 	cv::imshow("test",Il);
	cv::waitKey(1);
*/
       
    }
  

	

 


	

    dahengDevice.releaseImagesBuffer();
    dahengDevice.closeBinocular();
    dahengDevice.dahengDeviceCloseLib();


    return 0;
}


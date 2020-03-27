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
#include "include/CommonProcessingUnit.h"
#include "include/common.h"
#include "include/refineDisparity.h"
#include "include/socketImageTransfer.h"




//using namespace pcl;
using namespace std;
using namespace cv;
using namespace daheng;
using namespace CommonProcessingUnit;
using namespace StereoMatching;
using namespace RefineDisparity;

//pcl::visualization::PCLVisualizer viewer1("viewer_1");

queue<ImagePair> pairs;
mutex mu;  

bool working = true;

float cropFactor = 0.5;

cv::Mat lmapx, lmapy, rmapx, rmapy;
DahengDevice dahengDevice;

TCPServer tcpServer;

bool sendFlag = false;

//const string ipAddress = "192.168.101.95";
//const int port = 8888;


void cameraThread()
{
    while (working)
    {
        dahengDevice.acquisitionBinocularImages();
        
        Mat stereoImg(cv::Size(dahengDevice.stereoImgData.nWidth, dahengDevice.stereoImgData.nHeight), CV_8UC1, (void*)dahengDevice.stereoImgData.pImgBuf, cv::Mat::AUTO_STEP);
        int widthS = dahengDevice.stereoImgData.nWidth;
        int heightS = dahengDevice.stereoImgData.nHeight;
        Rect rectL(0, 0, widthS * cropFactor, heightS);
        Rect rectR(widthS * cropFactor, 0, widthS * cropFactor, heightS);
        
        Mat Il(stereoImg, rectR);
        Mat Ir(stereoImg, rectL);
        ImagePair imgs;
        imgs.imgL = Il.clone();
        imgs.imgR = Ir.clone();
        imgs.imgID = dahengDevice.stereoImgData.nFrameID%1000;
        mu.lock();
        if (pairs.size() > 5)
        {
            pairs.pop();
        }
        pairs.push(imgs);
        mu.unlock();

        //cout <<"cameraThread: "<< "leftImgData: "<< imgs.imgID << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

void receriveRequest()
{
    while(working)
    {
        string recvStr = tcpServer.receiveCommand();
		string depthCommand = "depth";
		string::size_type idx;
		idx = recvStr.find(depthCommand);
		if(idx == string::npos)
			std::cout<<"request command error!!!"<<std::endl;
		else
		{
			std::cout<<"send depth "<<std::endl;
			sendFlag = true;

		}
 			//tcpServer.sendDepthImg();
		//std::cout<<"iiiii:"<<recvStr<<std::endl;
    }

}






int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		std::cout<<"input format error!!!  example: ./execute ipaddress port cameraID"<<std::endl;
		return -1;
	}
	string ipAddress = argv[1];
 	int port = atoi(argv[2]);
    int cameraID =  atoi(argv[3]);
	
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
   tcpServer.openSocket(ipAddress,port);
   std::cout<<"connected ip: "<<ipAddress<<" port: "<<port<<" cameraID: "<<cameraID<<std::endl;
    //std::thread receiveThread(receriveRequest);

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
    int heightS = dahengDevice.stereoImgData.nHeight;
    Rect rectL(0, 0, widthS * cropFactor, heightS);
    Rect rectR(widthS * cropFactor, 0, widthS * cropFactor, heightS);
    Mat I1(stereoImg, rectL);
    Mat I2(stereoImg, rectR);

  
    ASSERT_MSG(!I1.empty() && !I2.empty(), "imread failed.");
    ASSERT_MSG(I1.size() == I2.size() && I1.type() == I2.type(), "input images must be same size and type.");
    ASSERT_MSG(I1.type() == CV_8U || I1.type() == CV_16U, "input image format must be CV_8U or CV_16U.");
    ASSERT_MSG(disp_size == 64 || disp_size == 128, "disparity size must be 64 or 128.");

    int width = I1.cols;
    int height = I1.rows;
    cout << I1.type() << endl;

    const int input_depth = I1.type() == CV_8U ? 8 : 16;
    const int input_bytes = input_depth * width * height / 8;
    const int output_depth = 8;
    const int output_bytes = output_depth * width * height / 8;

    sgm::StereoSGM sgm(width, height, disp_size, input_depth, output_depth, sgm::EXECUTE_INOUT_CUDA2CUDA);
    device_buffer d_I1(input_bytes), d_I2(input_bytes), d_disparity(output_bytes);
    cv::Mat disparity(height, width, output_depth == 8 ? CV_8U : CV_16U);

    
    bool flag = false;
    createRemapMat( lmapx,  lmapy,  rmapx,  rmapy, I2, cameraID);

    std::thread cameraAcquisition(cameraThread);
	

    while (1)
    {
       // cout << "test" << endl;
        begin = clock();
        //cout << "mian thread: "<< endl;
        if (!pairs.empty())
        {
			
            mu.lock();
            ImagePair imgs = pairs.front();
            pairs.pop();
            mu.unlock();
            if (!imgs.imgL.empty() && !imgs.imgR.empty())
            {
                Mat stereoImg;
                hconcat(imgs.imgL, imgs.imgR, stereoImg);
               

                //imwrite("../image/left.png", imgs.imgL);
                //imwrite("../image/right.png", imgs.imgR);
                // cv::waitKey(1);
				
                rectifyStereo(imgs.imgL, imgs.imgR, leftR, rightR, lmapx, lmapy, rmapx, rmapy);
		imwrite("left.png",leftR);
		imwrite("right.png",rightR);
				
                flag = true;
                //cv::Mat leftR = leftR.clone();
                //cv::Mat rightR = rightR.clone();
				const auto t1 = std::chrono::system_clock::now();
                cudaMemcpy(d_I1.data, leftR.data, input_bytes, cudaMemcpyHostToDevice);
                cudaMemcpy(d_I2.data, rightR.data, input_bytes, cudaMemcpyHostToDevice);
                sgm.execute(d_I1.data, d_I2.data, d_disparity.data);
                cudaMemcpy(disparity.data, d_disparity.data, output_bytes, cudaMemcpyDeviceToHost);
                if (disparity.empty())
                {
                    cout << "data empty" << endl;
                    break;
                }
                Mat result;
                //cv::medianBlur(disparity, result, 15);
                //cv::GaussianBlur(disparity, result, cv::Size(11, 11), 0, 0);
                //bilateralFilter(disparity, result, 11, 11 * 2, 11 / 2);

                //result = guidedFilter(imgs.imgL, disparity, r, eps);
                
                // cv::imwrite("../image/left.png", imgs.imgL);
                //cv::imwrite("../image/Original.png", disparity);
                //cv::imwrite("../image/afterFilter.png", result);

				
               
				
                //drawDisparity(disparity, duration, 128);
				const auto t2 = std::chrono::system_clock::now();
				const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
				//std::cout<<"comsumed time: "<<duration * 1e-3<<std::endl;
                //drawColorDisparity(disparity,  128);
				cv::Mat sendImg;
                hconcat(leftR, disparity, sendImg);              
				//std::cout<<sendImg.channels()<<std::endl;
                tcpServer.sendImage(sendImg);
		
				tcpServer.depth = disparity.clone();
               
                //Mat depth(leftR.rows, leftR.cols, CV_32FC1);
                //disparityToDepth(disparity, depth);

                

          

            //if (flag == 32)
            //{
             //   saveCloudPoint(depth, leftR);
           // }

            }
        }

    }

    cameraAcquisition.join();
    //receiveThread.join();
	

    dahengDevice.releaseImagesBuffer();
    dahengDevice.closeBinocular();
    dahengDevice.dahengDeviceCloseLib();
    tcpServer.closed();

    return 0;
}


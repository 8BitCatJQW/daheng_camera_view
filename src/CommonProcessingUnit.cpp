#include "../include/CommonProcessingUnit.h"


using namespace cv;

int user_data;

namespace CommonProcessingUnit
{ 
    template <class... Args>
    static std::string format_string(const char* fmt, Args... args)
    {
        const int BUF_SIZE = 1024;

        char buf[BUF_SIZE];
        std::snprintf(buf, BUF_SIZE, fmt, args...);
        return std::string(buf);
    }

    void createRemapMat(cv::Mat& lmapx, cv::Mat& lmapy, cv::Mat& rmapx, cv::Mat& rmapy, cv::Mat referenceimage,int cameraID)
    {
        Mat R1, R2, P1, P2, Q;
        Mat K1, K2, R;
        Vec3d T;
        Mat D1, D2;
		char* calib_file;
		if(cameraID ==1)
        	calib_file = "../stereo_parameter/cam_stereo.yml";
		else
 			calib_file = "../stereo_parameter/cam_stereo2.yml";

        cv::FileStorage fs1(calib_file, cv::FileStorage::READ);
        fs1["K1"] >> K1;
        fs1["K2"] >> K2;
        fs1["D1"] >> D1;
        fs1["D2"] >> D2;
        fs1["R"] >> R;
        fs1["T"] >> T;

        fs1["R1"] >> R1;
        fs1["R2"] >> R2;
        fs1["P1"] >> P1;
        fs1["P2"] >> P2;
        fs1["Q"] >> Q;

        
        cv::Mat imgU1, imgU2;

        cv::initUndistortRectifyMap(K1, D1, R1, P1, referenceimage.size(), CV_32F, lmapx, lmapy);
        cv::initUndistortRectifyMap(K2, D2, R2, P2, referenceimage.size(), CV_32F, rmapx, rmapy);

    }

    void rectifyStereo(const Mat& leftImg, const Mat& rightImg, Mat& leftR ,Mat& rightR,
        const cv::Mat& lmapx, const cv::Mat& lmapy, const cv::Mat& rmapx, const cv::Mat& rmapy)
    {

      
        cv::remap(leftImg, leftR, lmapx, lmapy, cv::INTER_LINEAR);
        cv::remap(rightImg, rightR, rmapx, rmapy, cv::INTER_LINEAR);

    }

    void drawDisparity(const cv::Mat&  disparity, const double& duration, const int& disp_size)
    {

        // draw results
        cv::Mat disparity_8u, disparity_color;
        //imwrite("../image/disparity.png", disparity);
 
        disparity.convertTo(disparity_8u, CV_8U, 255. / disp_size);

        cv::applyColorMap(disparity_8u, disparity_color, cv::COLORMAP_JET);

        //imwrite("../image/disparity_color.png", disparity_color);

        const double fps = 1e6 / duration;
        cv::putText(disparity_color, format_string("sgm execution time: %4.1f[ms] %4.1f[FPS]", duration/1e3, fps),

        cv::Point(50, 50), 2, 0.75, cv::Scalar(255, 255, 255));

        cv::imshow("disparity", disparity_color);

    }

    void preprocessingDisparity(const cv::Mat&  disparity, cv::Mat& result_filter)
    {

       ///* cv::medianBlur(disparity, result_filter, 7);*/
       // const int d = 6;
       // const float sigmaColor = 20;
       // const float sigmaSpace = 20;

       // // ��˹˫���˲�
       //Mat result_bilateral;
       //cv::bilateralFilter(disparity, result_bilateral, d, sigmaColor, sigmaSpace);
       ////result_filter = disparity;
       //// cv::imshow("disparity1", result_filter);



       // Mat thresh;

       // threshold(result_bilateral, thresh, 1, 255, 0);

       // //imshow("thresh", thresh);


       // Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));	//������
       // Mat open_result;	
       // morphologyEx(thresh, open_result, MORPH_OPEN,element);	

       // //imshow("������", open_result);
       // Mat close_result;	
       // morphologyEx(open_result, close_result, MORPH_CLOSE,element);
       // //imshow("������", close_result);
       // 
       // Mat result(disparity.rows, disparity.cols, CV_8UC1);

       // for (int j = 0; j < disparity.rows; ++j)
       // {
       //     for (int i = 0; i < disparity.cols; ++i)
       //     {
       //         if (open_result.at<uchar>(j, i) > 0)
       //             result.at<uchar>(j, i) = disparity.at<uchar>(j, i);
       //        
       //     }
       // }
       //
       // result_filter = result;


        

        cv::medianBlur(disparity, result_filter, 3);
        
      
      
    }

    void disparityToDepth(const cv::Mat& imgDisparity8U, cv::Mat& depth)
    {
        //Depth
        Mat Q;
        FileStorage fs("../stereo_parameter/cam_stereo.yml", FileStorage::READ);
        fs["Q"] >> Q;
        fs.release();
        reprojectImageTo3D(imgDisparity8U, depth, Q, false, -1);
    }

 

  

    int saveCloudPoint(const cv::Mat& depth, const cv::Mat& color)
    {
    //     int distanceFactor = 1000;
    //     Mat src_RGB;
    //     if (color.channels() != 3)
    //     {
    //         cvtColor(color, src_RGB, COLOR_GRAY2BGR);
    //         //imshow("rgb", src_RGB);
    //     }
    //     else
    //     {
    //         src_RGB = color;
    //     }

    //    if (color.empty() || depth.empty())
    //    {
    //        cout << "The image is empty, please check it!" << endl;
    //        return -1;
    //    }

    //    // Point cloud under camera coordinate system
    //     PointCloud<PointXYZRGB>::Ptr cloud(new PointCloud<PointXYZRGB>);
    //     for (int j = 0; j < depth.rows; ++j)
    //     {
    //         for (int i = 0; i < depth.cols; ++i)
    //         {
    //             auto d = depth.at<cv::Vec3f>(j, i)[2];
    //              if (d == INFINITY || d == 0 || d > 5)
    //                 continue;
    //             PointXYZRGB p;
    //             p.z = -d*1000; // Zc = baseline * f / (d + doffs)
    //             p.x = depth.at<Vec3f>(j, i)[0] * 1000; //XC to the right, Yc down as positive
    //             p.y = -depth.at<Vec3f>(j, i)[1] * 1000;

    //             //RGB
    //             p.b = src_RGB.ptr<uchar>(j)[i * 3];
    //             p.g = src_RGB.ptr<uchar>(j)[i * 3 + 1];
    //             p.r = src_RGB.ptr<uchar>(j)[i * 3 + 2];

    //             cloud->points.push_back(p);
    //         }
    //     }

    //     cloud->height = depth.rows;
    //     cloud->width = depth.cols;
    //     cloud->points.resize(cloud->height * cloud->width);
        
    //     pcl::PCLPointCloud2 cloud_blob;
    //     pcl::toPCLPointCloud2(*cloud, cloud_blob);

       
    //     pcl::io::savePLYFile("../image/cloud.ply", *cloud);
    //    // writer.write("test.ply", cloud_blob, Eigen::Vector4f::Zero(), Eigen::Quaternionf::Identity(), true, true);


    //     //viewer1.removeAllPointClouds();  // �Ƴ���ǰ���е���
    //     //viewer1.addPointCloud(cloud, "test");

    //     //viewer1.updatePointCloud(cloud, "test");
        
        return 0;
    }

   

    void drawColorDisparity(const cv::Mat&  disparity, const int& disp_size)
    {
        // draw results
        cv::Mat disparity_8u, disparity_color;
        disparity.convertTo(disparity_8u, CV_8U, 255. / disp_size);
        cv::applyColorMap(disparity_8u, disparity_color, cv::COLORMAP_JET);

        cv::imshow("disparity_color", disparity_color);
    }


    void sameScaleConversion(const cv::Mat& depth)
    {
        vector<cv::Mat> channels;
        cv::split(depth, channels);
        cv::Mat test = channels.at(2);
        cv::Mat matScale;
        test.convertTo(matScale, CV_8UC1, 255 / 5);
        // std::cout << depth_mat.channels() << std::endl;
        imshow("matScaleGray", matScale);
        cv::Mat imColor;
        cv::applyColorMap(matScale, imColor, cv::COLORMAP_JET);
        imshow("imColor", imColor);
    
    }








}

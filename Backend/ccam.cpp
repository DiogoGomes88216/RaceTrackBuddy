#include "ccam.h"
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>
#include <fstream>

CCam::CCam()
{

}

CCam::~CCam()
{
    closeCam();
}

void CCam::closeCam()
{
    cam.release();
}

void CCam::setTiming(string lap, string lastLap, string bestLap, string Delta, string totalTime)
{
    putText(lastFrame, "Last Lap: " + lastLap, Point(7,20), FONT_HERSHEY_DUPLEX, 0.5, Scalar(255,255,255),1);
    if(Delta[0] == '-')
        putText(lastFrame, "Delta: " + Delta, Point(237,20), FONT_HERSHEY_DUPLEX, 0.5, Scalar(36,255,0),1);
    else
        putText(lastFrame, "Delta: " + Delta, Point(237,20), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255),1);

    putText(lastFrame, "Best Lap: " + bestLap, Point(440,20), FONT_HERSHEY_DUPLEX, 0.5, Scalar(255,0,229),1);
    putText(lastFrame, "Lap: " + lap, Point(7,50), FONT_HERSHEY_DUPLEX, 0.7, Scalar(255,255,255),1);
    putText(lastFrame, "Total Time: " + totalTime, Point(130,50), FONT_HERSHEY_DUPLEX, 0.5, Scalar(255,255,255),1);
}

void CCam::setVA(string velocity, float gFy, float gFx)
{

    putText(lastFrame, velocity + " km/h", Point(360, 470), FONT_HERSHEY_DUPLEX, 1, Scalar(255,255,255),1);
    circle(	lastFrame, Point(590 + (gFx*40), 430 - (gFy*40)), 2, Scalar(0,255,255), -1, LINE_8, 0);
}

void CCam::setOverlay()
{
    rectangle(lastFrame, Point(0,0), Point(640,30), Scalar(0,0,0), -1, LINE_8, 0);
    rectangle(lastFrame, Point(0,30), Point(313,60), Scalar(0,0,0), -1, LINE_8, 0);
    rectangle(lastFrame, Point(353,440), Point(540,480), Scalar(0,0,0), -1, LINE_8, 0);
    rectangle(lastFrame, Point(540,380), Point(640,480), Scalar(0,0,0), -1, LINE_8, 0);
    circle(lastFrame, Point(590, 430), 50, Scalar(0,26,255), 1, LINE_8, 0);
    circle(lastFrame, Point(590, 430), 40, Scalar(0,26,255), 1, LINE_8, 0);
    circle(lastFrame, Point(590, 430), 30, Scalar(0,26,255), 1, LINE_8, 0);
    circle(lastFrame, Point(590, 430), 20, Scalar(0,26,255), 1, LINE_8, 0);
    circle(lastFrame, Point(590, 430), 10, Scalar(0,26,255), 1, LINE_8, 0);
    putText(lastFrame, "G", Point(585, 435), FONT_HERSHEY_DUPLEX, 0.5, Scalar(255,26,0),1);
}


bool CCam::openCam()
{
    cam.open(0, cv::CAP_V4L2);
    if (!cam.isOpened())
        return false;
    cam.set(CAP_PROP_FRAME_WIDTH, Width);
    cam.set(CAP_PROP_FRAME_HEIGHT, Height);
    cam.set(CAP_PROP_FPS, FPS);
    return true;

}

void CCam::catchFrame()
{
  cam.read(lastFrame);
  if (lastFrame.empty()) {
      cout << "ERROR! blank frame grabbed\n";
  }
}

CVideo::CVideo()
{

}

CVideo::~CVideo()
{
    closeVideo();
}

bool CVideo::open(string Date, string Time)
{
    try
     {
         const string videoPath = "/mnt/usb/Videos/" + Date + "_" + Time + ".avi";
         int format = video.fourcc('H','2','6','4');
         Size S = Size(Width, Height);

         video.open(videoPath, format, 30, S, true);
     }
     catch(cv::Exception& e)
     {
         const char* err_msg = e.what();
         std::cout << err_msg << std::endl;
     }
     if (!video.isOpened())
         return false;
     return true;
}

void CVideo::writeFrame(CCam *cam)
{
    video.write(cam->lastFrame);
}

void CVideo::closeVideo()
{
    video.release();
}

#ifndef CCAM_H
#define CCAM_H
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <iostream>

#define Width 640
#define Height 480
#define FPS 30

using namespace cv;
using namespace std;

class CCam
{
public:
    CCam();
    ~CCam();
    bool openCam();
    void catchFrame();
    void closeCam();
    void setTiming(string lap, string lastLap, string bestLap, string Delta, string totalTime);
    void setVA(string velocity, float gFy, float gFx);
    void setOverlay();
    Mat lastFrame;

private:
    VideoCapture cam;
};

class CVideo{
public:
    CVideo();
    ~CVideo();

    bool open(string Date, string Time);
    void writeFrame(CCam *cam);
    void closeVideo();
private:
     VideoWriter video;
};

#endif // CCAM_H

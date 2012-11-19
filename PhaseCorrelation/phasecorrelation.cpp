//Remco Tukker, 2012

#include "phasecorrelation.h"

#include <opencv/cxcore.h>
//#include <opencv2/imgproc.hpp> //this one should do the trick for cvLogPolar, fix my system so that i can remove next line..
#include <opencv2/opencv.hpp>

using namespace cv;

PhaseCorrelation::PhaseCorrelation()
{

}

void PhaseCorrelation::insertFrame(const Mat& nextFrame)  //frame should be 640*480 for now!
{
    //make place for the new frame and move the previous frame to the old frame variables
    swap(oldFramePolar, newFramePolar); //is swap actually a good idea here?
    swap(oldFrame, newFrame);

    newFramePolar.release();        //make the new ones empty, so that we know whether they are filled and up to date or not
    nextFrame.convertTo(newFrame, CV_32F, 1.0/255);

    return;
}


Vec4f PhaseCorrelation::findCorrelation()
{
    if (oldFrame.empty()) return Vec4f(0,0,0,0);

    // first get the scaling and rotation
    Point2d rot = findRotation();

    //use this rotation and scaling to calculate a new version of newFrame or oldFrame, depending on which one is bigger
    Point2d trans;
    if (rot.x > 1)  // oldFrame is bigger than newFrame, so turn oldFrame
    {
        Mat rotMat = getRotationMatrix2D( Point2f(oldFrame.size().width/2, oldFrame.size().height/2), rot.y, rot.x);
        Mat oldRotated;
        warpAffine(oldFrame, oldRotated, rotMat, Size(240,240) );

        trans = phaseCorrelate(oldRotated, newFrame(Rect(200, 120, 240, 240)));
    }
    else   // newFrame is bigger than oldFrame, so turn newFrame
    {

        Mat rotMat = getRotationMatrix2D( Point2f(newFrame.size().width/2, newFrame.size().height/2), rot.y, rot.x);
        Mat newRotated;
        //warpAffine(newFrame, newRotated, rotMat, Size(240,240) ); // the size argument just gives left top corner, not middle!
        warpAffine(newFrame, newRotated, rotMat, newFrame.size() );

        imshow("srtg", newRotated);

        //trans = phaseCorrelate(oldFrame(Rect(200, 120, 240, 240)), newRotated);
        trans = phaseCorrelate(oldFrame, newRotated);
    }

    return Vec4f(trans.x,trans.y,rot.x,rot.y);
}

Point2d PhaseCorrelation::findRotation()
{
    if (oldFrame.empty()) return Point2d(0,0);

    if (oldFramePolar.empty())
    {
        calculatePolar(oldFrame, oldFramePolar);
    }

    if (newFramePolar.empty())
    {
        calculatePolar(newFrame, newFramePolar);
    }

    Point2d result = phaseCorrelate(oldFramePolar, newFramePolar);

    // reverse logpolar transform
    result.y = result.y * 360.0 / (float) oldFramePolar.size().height ; //in degrees obviously
    //result[0] = exp result[0]  *  ....

    result.x = 1;
    result.y = 0;

    return result;
}

Point2d PhaseCorrelation::findTranslation()
{
    if (oldFrame.empty() ) return Point2d(0,0);

    return phaseCorrelate(oldFrame, newFrame);
}


void PhaseCorrelation::calculatePolar(const Mat& src, Mat& dst)
{
    //get the logpolar representation of src Mat; this stuff is _slow_
    Mat src2;
    src.convertTo(src2, CV_8U, 255);
    Mat frameLogPolar;
    IplImage img = src2;
    IplImage* img2 = &img;
    IplImage* logPolar = cvCreateImage( cvGetSize(img2), 8, 1  );
    //cvLogPolar should go faster with a fixed mapping and/or no interpolation; also, image size will change!
    cvLogPolar( img2, logPolar, cvPoint2D32f(src.size().width/2, src.size().height/2), 80, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
    Mat(logPolar).colRange(Range(0,420)).convertTo(dst, CV_32F, 1.0/255);

}



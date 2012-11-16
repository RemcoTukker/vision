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
    swap(oldFramePolarDFT, newFramePolarDFT); //is swap actually a good idea here?
    swap(oldFrameDFT, newFrameDFT);
    swap(oldFrame, newFrame);

    newFrameDFT.release();        //make the new ones empty, so that we know whether they are filled and up to date or not
    newFramePolarDFT.release();
    nextFrame.copyTo(newFrame);

    return;
}


Vec4f PhaseCorrelation::findCorrelation()
{
    if (oldFrame.empty()) return Vec4f(0,0,0,0);

    // first get the scaling and rotation
    Vec2f rot = findRotation();

    //use this rotation and scaling to calculate a new version of newFrame or oldFrame, depending on which one is bigger
    Vec2f trans;
    if (rot[0] > 1)  // oldFrame is bigger than newFrame, so turn oldFrame
    {
        Mat rotMat = getRotationMatrix2D( Point2f(oldFrame.size().width/2, oldFrame.size().height/2), rot[1], rot[0]);
        Mat oldRotated;
        warpAffine(oldFrame, oldRotated, rotMat, Size(240,240) );

        Mat dftready;
        oldRotated.convertTo(dftready, CV_32F, 1.0/255);
        calculateDFT(dftready, oldRotatedDFT);

        //dftready = newFrame.colRange().rowRange();

        //TODO make only dft of part of the image
        if (newFrameDFT.empty())
        {
            Mat dftready;
            newFrame.convertTo(dftready, CV_32F, 1.0/255);
            calculateDFT(dftready, newFrameDFT);
        }

        trans = crossPowerSpectrumPeak(oldRotatedDFT, newCroppedDFT);
    }
    else   // newFrame is bigger than oldFrame, so turn newFrame
    {
        if (oldFrameDFT.empty())
        {
            Mat dftready;
            oldFrame.convertTo(dftready, CV_32F, 1.0/255);
            calculateDFT(dftready, oldFrameDFT);
        }

        //trans = crossPowerSpectrumPeak(oldCroppedDFT, newRotatedDFT);
    }

    return Vec4f(trans[0],trans[1],rot[0],rot[1]);
}

Vec2f PhaseCorrelation::findRotation()
{
    if (oldFrame.empty()) return Vec2f(0,0);

    if (oldFramePolarDFT.empty())
    {
        calculatePolarDFT(oldFrame, oldFramePolarDFT);
    }

    if (newFramePolarDFT.empty())
    {
        calculatePolarDFT(newFrame, newFramePolarDFT);
    }

    Vec2f result = crossPowerSpectrumPeak(oldFramePolarDFT, newFramePolarDFT);

    // reverse logpolar transform
    result[1] = result[1] * 360.0 / (float) oldFramePolarDFT.size().height ; //in degrees obviously
    //result[0] = exp result[0]  *  ....

    return result;
}

Vec2f PhaseCorrelation::findTranslation()
{
    if (oldFrame.empty() ) return Vec2f(0,0);

    if (oldFrameDFT.empty())
    {
        Mat dftready;
        oldFrame.convertTo(dftready, CV_32F, 1.0/255);
        calculateDFT(dftready, oldFrameDFT);
    }

    if (newFrameDFT.empty())
    {
        Mat dftready;
        newFrame.convertTo(dftready, CV_32F, 1.0/255);
        calculateDFT(dftready, newFrameDFT);
    }

    return crossPowerSpectrumPeak(oldFrameDFT, newFrameDFT);
}

Vec2f PhaseCorrelation::crossPowerSpectrumPeak(const Mat& dft1, const Mat& dft2)
{

    Mat complexI;
    mulSpectrums(dft1, dft2, complexI, 0, true); //do the multiplications

    Mat images[2];

    /* //were supposed to do this, but it messes up result?
    split( complexI, images ); //images[0] real, images[1] imaginary

    // Compute the magnitude of the spectrum components: Mag = sqrt(Re^2 + Im^2)
    Mat imageMag, imageImMag;
    pow( images[0], 2.0, imageMag );

    pow( images[1], 2.0, imageImMag );
    add( imageMag, imageImMag, imageMag );
    pow( imageMag, 0.5, imageMag );

    divide( images[0], imageMag + 2.0, images[0]);
    divide( images[1], imageMag + 2.0, images[1]);

    merge(images, 2, complexI);
    */


    // third step: transform back and find peak

    idft(complexI, complexI);  //transform back
    split(complexI, images);

    //find max
    Point minLoc, maxLoc;
    double minVal, maxVal;
    minMaxLoc(images[0], &minVal, &maxVal, &minLoc, &maxLoc);

    //TODO: get subpixel accuracy here!

    int width = images[0].size().width;
    int height = images[0].size().height;

    if (maxLoc.x > (width / 2))
        maxLoc.x = maxLoc.x - width;

    if (maxLoc.y > (height / 2))
        maxLoc.y = maxLoc.y - height;

    return Vec2f(maxLoc.x, maxLoc.y);

}

void PhaseCorrelation::calculatePolarDFT(const Mat& src, Mat& dst)
{
    //get the logpolar representation of it to fill newFramePolarDFT; this stuff is _slow_ , thats why its behind the if
    Mat frameLogPolar;
    IplImage img = src;
    IplImage* img2 = &img;
    IplImage* logPolar = cvCreateImage( cvGetSize(img2), 8, 1  );
    //cvLogPolar should go faster with a fixed mapping and/or no interpolation; also, image size will change!
    cvLogPolar( img2, logPolar, cvPoint2D32f(src.size().width/2, src.size().height/2), 80, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
    Mat(logPolar).colRange(Range(0,420)).convertTo(frameLogPolar, CV_32F, 1.0/255);
    //NB colRange to get rid of the black corners that mess up the FFT result (?)

    //finally, calculate the DFT of the logpolar representation of the image
    calculateDFT(frameLogPolar, dst);

}

void PhaseCorrelation::calculateDFT(const Mat& src, Mat& dst)
{
    Mat planes[] = {src, Mat::zeros(src.size(), CV_32F)}; //make it 2 channel for FFT
    Mat complexI;
    merge(planes, 2, complexI );
    dft(complexI, dst);  // maybe we can speed up here because we have only real input?
}


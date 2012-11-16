//Remco Tukker, 2012

#include "phasecorrelation.h"

#include <opencv/cxcore.h>
//#include <opencv2/imgproc.hpp> //this one should do the trick for cvLogPolar, fix my system so that i can remove next line..
#include <opencv2/opencv.hpp>

using namespace cv;

PhaseCorrelation::PhaseCorrelation(bool withTranslation, bool withRotation) //default is true for both
{
    withRot = withRotation;
    withTrans = withTranslation;
}

void PhaseCorrelation::insertFrame(const Mat& nextFrame)  //frame should be 640*480 for now!
{
    //make place for the new frame and move the previous frame to the old frame variables
    swap(oldFramePolarDFT, newFramePolarDFT);
    swap(oldFrameDFT, newFrameDFT);

    if (withRot)
    {
        //get the logpolar representation of it to fill newFramePolarDFT; this stuff is _slow_ , thats why its behind the if
        Mat newFrameLogPolar;
        IplImage img = nextFrame;
        IplImage* src = &img;
        IplImage* dst = cvCreateImage( cvGetSize(src), 8, 1  );
        //cvLogPolar should go faster with a fixed mapping and/or no interpolation; also, image size will change!
        cvLogPolar( src, dst, cvPoint2D32f(nextFrame.size().width/2,nextFrame.size().height/2), 80, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
        Mat(dst).colRange(Range(0,420)).convertTo(newFrameLogPolar, CV_32F, 1.0/255);
        //NB colRange to get rid of the black corners that mess up the FFT result (?)

        //finally, calculate the DFT of the logpolar representation of the image
        Mat planes[] = {newFrameLogPolar, Mat::zeros(newFrameLogPolar.size(), CV_32F)}; //make it complex for FFT
        Mat complexI;
        merge(planes, 2, complexI );

        dft(complexI, complexI);  // maybe we can speed up here because we have only real input?
        complexI.copyTo(newFramePolarDFT);
    }

    if (withTrans)
    {
        // fill newFrameDFT
       // Mat planes2[] = {newFrame, Mat::zeros(newFrame.size(), CV_32F)};
       // Mat complex2;
       // merge(planes2, 2, complex2);
       // dft(complex2, complex2);
       // complex2.copyTo(newFrameDFT);
    }

    return;
}


Vec4f PhaseCorrelation::findCorrelation()
{
    // first get the scaling and rotation
    Vec2f rot = findRotation();

    //oh dear, now we have to calculate a different DFT after all; TODO manage stuff a bit more intelligently...

    return Vec4f(0,0,0,0);
}

Vec2f PhaseCorrelation::findRotation()
{
    if (oldFramePolarDFT.empty()) return Point2f(0,0);  //if withRot == false, oldFramePolarDFT will be empty anyway

    Vec2f result = crossPowerSpectrumPeak(oldFramePolarDFT, newFramePolarDFT);

    // reverse logpolar transform
    result[1] = result[1] * 360.0 / (float) oldFramePolarDFT.size().height ; //in degrees obviously


    return result;
}

Vec2f PhaseCorrelation::findTranslation()
{
    if (oldFrameDFT.empty() ) return Point2f(0,0); //if withTrans == false, oldFrameDFT will be empty anyway

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

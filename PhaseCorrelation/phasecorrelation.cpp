//Remco Tukker, 2012

#include "phasecorrelation.h"


using namespace cv;

PhaseCorrelation::PhaseCorrelation()
{
    //ctor
}

void PhaseCorrelation::insertFrame(Mat& nextFrame)  //frame should be 640*480 for now!
{
    //make place for the new frame and move the previous frame to the old frame variables
    swap(oldFrame, newFrame);
    swap(oldFrameLogPolar, newFrameLogPolar);
    swap(oldFramePolarDFT, newFramePolarDFT);

    //fill newFrame with received frame
    nextFrame.copyTo(newFrame);
    //get the logpolar representation of it to fill newFrameLogPolar again
    IplImage img = newFrame;
    IplImage* src = &img;
    IplImage* dst = cvCreateImage( cvGetSize(src), 8, 1  );
    //cvLogPolar should go faster with a fixed mapping and/or no interpolation; also, image size will change!
    cvLogPolar( src, dst, cvPoint2D32f(newFrame.size().width/2,newFrame.size().height/2), 80, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
    Mat(dst).colRange(Range(0,420)).convertTo(newFrameLogPolar, CV_32F, 1.0/255);
    //NB colRange to get rid of the black corners that mess up the FFT result (?)

    //finally, calculate the DFT of the logpolar representation of the image
    Mat planes[] = {newFrameLogPolar, Mat::zeros(newFrameLogPolar.size(), CV_32F)}; //make it complex for FFT
    Mat complexI;
    merge(planes, 2, complexI );

    dft(complexI, complexI);  // maybe we can speed up here because we have only real input?
    complexI.copyTo(newFramePolarDFT);

    return;
}

Point2f PhaseCorrelation::calculateCorrelation()
{

    if (oldFramePolarDFT.empty() ) return Point2f(0,0);  //check if we have two frames

    //imshow("rg", oldFrameLogPolar);

    Mat complexI;
    mulSpectrums(oldFramePolarDFT, newFramePolarDFT, complexI, 0, true); //do the multiplications

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

    idft(complexI, complexI);  //transform back
    split(complexI, images);

    //find max in real part
    Point minLoc, maxLoc;
    double minVal, maxVal;
    minMaxLoc(images[0], &minVal, &maxVal, &minLoc, &maxLoc);

    //cout << maxLoc.x << " " << maxLoc.y << endl;
    //cout << maxVal << " " << minVal << endl;

    //Mat result1;
    //images[0].convertTo(result1, CV_8U, 1.0 / maxVal);
    //imshow("peak values", result1);

    Point2f result = maxLoc;

    return result;
}

//Remco Tukker, 2012
//
//TODO
//
// processing cost
// *make logpolar conversion faster
// *add zero padding to make FFT faster / get more accurate result (?)
// *can we speed up fft because we have only real input?
//
// result
// *check whether it might be better to use a different center for logpolar conversion
// *maybe use Hanning window, do we care about edge effects?
// *find out whats going wrong in fourier space, if anything
// *add translation, zoom
// *optionally multiple with prior to get more reliable result
// *get subpixel result by fitting peak
// *check if corners in logpolar coordinates do indeed mess up fft result (and what about zero padding?)
//
// usability
// *check for image size and type and change parameters accordingly
//
// description:
// feed this class a frame (grayscale, 640*480, type?) when it comes in with insertFrame. Then call
// calculateCorrelation to find the rotation and zooming between the frame you just fed it and the frame
// you fed it before.


#ifndef PHASECORRELATION_H
#define PHASECORRELATION_H

#include <opencv2/core/core.hpp>

using namespace cv;

class PhaseCorrelation
{
    public:
        PhaseCorrelation();
        void insertFrame(Mat& nextFrame);
        Point2f calculateCorrelation();

    protected:
    private:
        Mat oldFrame;
        Mat newFrame;
        Mat oldFrameLogPolar;
        Mat newFrameLogPolar;
        Mat oldFramePolarDFT;
        Mat newFramePolarDFT;
};

#endif // PHASECORRELATION_H

//Remco Tukker, 2012
//
//TODO
//
// processing cost
// -make logpolar conversion faster
// -can we do without the inverse FFT? (see wikipedia on phase correlation)*
// -can we use the fourier shift theorem to acquire sub-pixel result? (see wikipedia on phase correlation)*
//
// result (in general: take a good & thorough look at original paper again)
// -implement zoom
// -is it better to use the fft of the logpolar representation or the logpolar representation of the fft?
// -check whether it might be better to use a different center for logpolar conversion
// -fix ambiguity between clockwise and anticlockwise rotations, related to ^ (see original paper)
// -maybe use Hanning window, do we care about edge effects?
// -optionally multiple with prior to get more reliable result *
// -check if corners in logpolar coordinates do indeed mess up fft result (and what about zero padding?)
// -fix window in rotated images to actually maximalize area, instead of the conservative square chosen now
//
// usability
// -check for image size and type and change parameters accordingly
// -maybe even check every frame whether frame type is still the same
//
// * Improvement requires opencv phasecorrelate to be adapted
//
// description:
// feed this class a frame (grayscale, 640*480, CV_8UC1) when it comes in with insertFrame. Then call
// calculateCorrelation to find the rotation and zooming between the frame you just fed it and the frame
// you fed it before.
//
//this covers most use scenarios perfectly:
//    -get the whole thing
//    -or get only translation
//scenarios that are not covered perfectly:
//    -don't always use successive frames; to cover that, build some seperate functions from the code below



#ifndef PHASECORRELATION_H
#define PHASECORRELATION_H

#include <opencv2/core/core.hpp>

using namespace cv;

class PhaseCorrelation
{
    public:
        PhaseCorrelation();
        void insertFrame(const Mat& nextFrame);
        Vec4f findCorrelation(); //the whole package
        Point2d findRotation();    //only calculate rotation and scaling
        Point2d findTranslation(); //only calculate translation
        void calculatePolar(const Mat& src, Mat& dst);
    protected:
    private:
        Mat oldFramePolar;
        Mat newFramePolar;
        Mat oldFrame;
        Mat newFrame;
};

#endif // PHASECORRELATION_H

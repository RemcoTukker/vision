//Remco Tukker, 2012
//
//TODO
//
// processing cost
// *make logpolar conversion faster
// *add zero padding to make FFT faster / get more accurate result (?)
// *can we speed up fft because we have only real input?
// *can we do without the inverse FFT? (see wikipedia on phase correlation)
// *can we use the fourier shift theorem to acquire sub-pixel result? (see wikipedia on phase correlation)
//
// result
// *check whether it might be better to use a different center for logpolar conversion
// *fix ambiguity between clockwise and anticlockwise rotations, related to ^ (see original paper)
// *maybe use Hanning window, do we care about edge effects?
// *find out whats going wrong in fourier space (logpolar), if anything
// *add translation, zoom
// *optionally multiple with prior to get more reliable result
// *get subpixel result by fitting peak
// *check if corners in logpolar coordinates do indeed mess up fft result (and what about zero padding?)
//
// usability
// *check for image size and type and change parameters accordingly
// *maybe even check every frame whether frame type is still the same
//
// description:
// feed this class a frame (grayscale, 640*480, type?) when it comes in with insertFrame. Then call
// calculateCorrelation to find the rotation and zooming between the frame you just fed it and the frame
// you fed it before.
//
//this covers most use scenarios perfectly:
//    *get the whole thing
//    *or get only translation
//scenarios that are not covered perfectly:
//    *don't always use successive frames; to cover that, build some seperate functions from the code below



#ifndef PHASECORRELATION_H
#define PHASECORRELATION_H

#include <opencv2/core/core.hpp>

using namespace cv;

class PhaseCorrelation
{
    public:
        PhaseCorrelation(bool withTranslation = true, bool withRotation = true);
        void insertFrame(const Mat& nextFrame);
        Vec4f findCorrelation(); //the whole package
        Vec2f findRotation();    //only calculate rotation and scaling
        Vec2f findTranslation(); //only calculate translation
    protected:
    private:
        Vec2f crossPowerSpectrumPeak(const Mat& dft1, const Mat& dft2);
        Mat oldFramePolarDFT;
        Mat newFramePolarDFT;
        Mat oldFrameDFT;
        Mat newFrameDFT;
        bool withRot;
        bool withTrans;
};

#endif // PHASECORRELATION_H

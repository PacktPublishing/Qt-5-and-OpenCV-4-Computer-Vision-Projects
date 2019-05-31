#include "affine_plugin.h"

using namespace cv;

QString AffinePlugin::name()
{
    return "Affine";
}

void AffinePlugin::edit(const cv::Mat &input, cv::Mat &output)
{

    Point2f triangleA[3];
    Point2f triangleB[3];

    triangleA[0] = Point2f(0 , 0);
    triangleA[1] = Point2f(1 , 0);
    triangleA[2] = Point2f(0 , 1);

    triangleB[0] = Point2f(0, 0);
    triangleB[1] = Point2f(1, 0);
    triangleB[2] = Point2f(1, 1);

    Mat affineMatrix = getAffineTransform(triangleA, triangleB);
    Mat result;
    warpAffine(
        input, result,
        affineMatrix, input.size(), // output image size, same as input
        INTER_CUBIC, // Interpolation method
        BORDER_CONSTANT  // Extrapolation method
        //BORDER_WRAP  // Extrapolation method
    );

    output = result;
}

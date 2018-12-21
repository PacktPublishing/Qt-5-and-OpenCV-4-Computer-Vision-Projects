#include "rotate_plugin.h"

QString RotatePlugin::name()
{
    return "Rotate";
}

void RotatePlugin::edit(const cv::Mat &input, cv::Mat &output)
{
    double angle = 45.0;
    double scale = 1.0;
    cv::Point2f center = cv::Point(input.cols/2, input.rows/2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle, scale);

    cv::Mat result;
    cv::warpAffine(input, result,
        rotateMatrix, input.size(),
        cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    output = result;
}

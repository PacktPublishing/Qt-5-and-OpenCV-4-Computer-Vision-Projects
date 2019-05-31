#include "erode_plugin.h"

QString ErodePlugin::name()
{
    return "Erode";
}

void ErodePlugin::edit(const cv::Mat &input, cv::Mat &output)
{
    erode(input, output, cv::Mat());
}

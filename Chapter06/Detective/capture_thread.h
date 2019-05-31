// mode: c++
#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/dnn.hpp"

using namespace std;

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(int camera, QMutex *lock);
    explicit CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) {running = run; };
    void takePhoto() {taking_photo = true; }

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void photoTaken(QString name);

private:
    void takePhoto(cv::Mat &frame);
    void detectObjects(cv::Mat &frame);
    void detectObjectsDNN(cv::Mat &frame);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    int frame_width, frame_height;

    // take photos
    bool taking_photo;

    // object detection
    cv::CascadeClassifier *classifier;

    cv::dnn::Net net;
    vector<string> objectClasses;
};

#endif // CAPTURE_THREAD_H

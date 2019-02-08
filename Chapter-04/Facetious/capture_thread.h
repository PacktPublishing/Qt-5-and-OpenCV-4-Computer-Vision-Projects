// mode: c++
#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"

using namespace std;

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(int camera, QMutex *lock);
    explicit CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) {running = run; };

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    int frame_width, frame_height;
};

#endif // CAPTURE_THREAD_H

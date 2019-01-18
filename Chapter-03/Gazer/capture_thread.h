// mode: c++
#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(int camera, QMutex *lock);
    explicit CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) {running = run; };
    void startCalcFPS() {fps_calculating = true; };

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(int fps);

private:
    void calculateFPS(cv::VideoCapture &cap);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    // FPS calculating
    bool fps_calculating;
    int fps;
};

#endif // CAPTURE_THREAD_H

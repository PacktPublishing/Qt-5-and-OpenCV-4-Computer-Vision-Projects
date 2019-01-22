// mode: c++
#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(int camera, QMutex *lock);
    explicit CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) {running = run; };
    void startCalcFPS() {fps_calculating = true; };
    enum VideoSavingStatus {
                            STARTING,
                            STARTED,
                            STOPPING,
                            STOPPED
    };

    void setVideoSavingStatus(VideoSavingStatus status) {video_saving_status = status; };

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(int fps);
    void videoSaved(QString name);

private:
    void calculateFPS(cv::VideoCapture &cap);
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    // FPS calculating
    bool fps_calculating;
    int fps;

    // video saving
    int frame_width, frame_height;
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;
};

#endif // CAPTURE_THREAD_H

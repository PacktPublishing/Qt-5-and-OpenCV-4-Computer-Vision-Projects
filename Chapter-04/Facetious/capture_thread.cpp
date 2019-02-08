#include <QTime>
#include <QDebug>

#include "utilities.h"
#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    frame_width = frame_height = 0;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    frame_width = frame_height = 0;
}

CaptureThread::~CaptureThread() {
}

void CaptureThread::run() {
    running = true;
    cv::VideoCapture cap(cameraID);
    // cv::VideoCapture cap("/home/kdr2/Videos/WIN_20190123_20_14_56_Pro.mp4");
    cv::Mat tmp_frame;

    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    while(running) {
        cap >> tmp_frame;
        if (tmp_frame.empty()) {
            break;
        }

        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        emit frameCaptured(&frame);
    }
    cap.release();
    running = false;
}

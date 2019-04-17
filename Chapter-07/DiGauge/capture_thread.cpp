#include <QTime>
#include <QDebug>

#include "utilities.h"
#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;
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
        if(taking_photo) {
            takePhoto(tmp_frame);
        }

#ifdef TIME_MEASURE
        int64 t0 = cv::getTickCount();
#endif
        detectObjectsDNN(tmp_frame);

#ifdef TIME_MEASURE
        int64 t1 = cv::getTickCount();
        double t = (t1-t0) * 1000 /cv::getTickFrequency();
        qDebug() << "Decteing time on a single frame: " << t <<"ms";
#endif

        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        emit frameCaptured(&frame);
    }
    cap.release();
    running = false;
}


void CaptureThread::takePhoto(cv::Mat &frame)
{
    QString photo_name = Utilities::newPhotoName();
    QString photo_path = Utilities::getPhotoPath(photo_name, "jpg");
    cv::imwrite(photo_path.toStdString(), frame);
    emit photoTaken(photo_name);
    taking_photo = false;
}

static void decodeOutLayers(
    cv::Mat &frame, const vector<cv::Mat> &outs,
    vector<int> &outClassIds,
    vector<float> &outConfidences,
    vector<cv::Rect> &outBoxes
);

void CaptureThread::detectObjectsDNN(cv::Mat &frame)
{
    int inputWidth = 416;
    int inputHeight = 416;

    if (net.empty()) {
        // give the configuration and weight files for the model
        string modelConfig = "data/yolov3.cfg";
        string modelWeights = "data/yolov3.weights";
        net = cv::dnn::readNetFromDarknet(modelConfig, modelWeights);
        // net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        // net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        objectClasses.clear();
        string name;
        string namesFile = "data/coco.names";
        ifstream ifs(namesFile.c_str());
        while(getline(ifs, name)) objectClasses.push_back(name);
    }

    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(inputWidth, inputHeight), cv::Scalar(0, 0, 0), true, false);

    net.setInput(blob);

    // forward
    vector<cv::Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());

#ifdef TIME_MEASURE
    vector<double> layersTimes;
    double freq = cv::getTickFrequency() / 1000;
    double t = net.getPerfProfile(layersTimes) / freq;
    qDebug() << "YOLO: Inference time on a single frame: " << t <<"ms";
#endif

    // remove the bounding boxes with low confidence
    vector<int> outClassIds;
    vector<float> outConfidences;
    vector<cv::Rect> outBoxes;
    decodeOutLayers(frame, outs, outClassIds, outConfidences, outBoxes);

    for(size_t i = 0; i < outClassIds.size(); i ++) {
        cv::rectangle(frame, outBoxes[i], cv::Scalar(0, 0, 255));

        // get the label for the class name and its confidence
        string label = objectClasses[outClassIds[i]];
        label += cv::format(":%.2f", outConfidences[i]);

        // display the label at the top of the bounding box
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int left = outBoxes[i].x, top = outBoxes[i].y;
        top = max(top, labelSize.height);
        cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255));
    }
}

void decodeOutLayers(
    cv::Mat &frame, const vector<cv::Mat> &outs,
    vector<int> &outClassIds,
    vector<float> &outConfidences,
    vector<cv::Rect> &outBoxes
)
{
    float confThreshold = 0.5; // confidence threshold
    float nmsThreshold = 0.4;  // non-maximum suppression threshold

    vector<int> classIds;
    vector<float> confidences;
    vector<cv::Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i) {
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            // get the value and location of the maximum score
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold)
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    // non maximum suppression
    vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        outClassIds.push_back(classIds[idx]);
        outBoxes.push_back(boxes[idx]);
        outConfidences.push_back(confidences[idx]);
    }
}

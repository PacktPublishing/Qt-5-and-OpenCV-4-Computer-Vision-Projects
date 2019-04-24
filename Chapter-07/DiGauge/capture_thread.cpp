#include <utility>
#include <algorithm>

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
    vector<cv::Rect> &outBoxes
);
void distanceBirdEye(cv::Mat &frame, vector<cv::Rect> &cars);

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
    vector<cv::Rect> outBoxes;
    decodeOutLayers(frame, outs, outBoxes);

    for(size_t i = 0; i < outBoxes.size(); i ++) {
        cv::rectangle(frame, outBoxes[i], cv::Scalar(0, 0, 255));
    }
    distanceBirdEye(frame, outBoxes);
}

void decodeOutLayers(
    cv::Mat &frame, const vector<cv::Mat> &outs,
    vector<cv::Rect> &outBoxes
)
{
    float confThreshold = 0.65; // confidence threshold
    float nmsThreshold = 0.4;  // non-maximum suppression threshold

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
            if (classIdPoint.x != 2) // not a car!
                continue;
            if (confidence > confThreshold)
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

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
        outBoxes.push_back(boxes[idx]);
    }
}

void distanceBirdEye(cv::Mat &frame, vector<cv::Rect> &cars)
{
    vector<int> length_of_cars;
    vector<pair<int, int>> endpoints;
    vector<pair<int, int>> cars_merged;

    for (auto car: cars) {
        length_of_cars.push_back(car.width);
        endpoints.push_back(make_pair(car.x, 1));
        endpoints.push_back(make_pair(car.x + car.width, -1));
    }

    sort(length_of_cars.begin(), length_of_cars.end());
    int length = length_of_cars[cars.size() / 2];
    sort(
        endpoints.begin(), endpoints.end(),
        [](pair<int, int> a, pair<int, int> b) {
            return a.first < b.first;
        }
    );

    int flag = 0, start = 0;
    for (auto ep: endpoints) {
        flag += ep.second;
        if (flag == 1 && start == 0) { // a start
            start = ep.first;
        } else if (flag == 0) { // an end
            cars_merged.push_back(make_pair(start, ep.first));
            start = 0;
        }
    }

    for (size_t i = 1; i < cars_merged.size(); i++) {
        int x1 = cars_merged[i - 1].second; // head of car, start of spacing
        int x2 = cars_merged[i].first; // end of another car, end of spacing
        cv::line(frame, cv::Point(x1, 0), cv::Point(x1, frame.rows), cv::Scalar(0, 255, 0), 2);
        cv::line(frame, cv::Point(x2, 0), cv::Point(x2, frame.rows), cv::Scalar(0, 0, 255), 2);
        float distance = (x2 - x1) * (5.0 / length);

        // display the label at the top of the bounding box
        string label = cv::format("%.2f m", distance);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int label_x = (x1 + x2) / 2 - (labelSize.width / 2);
        cv::putText(frame, label, cv::Point(label_x, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
    }
}

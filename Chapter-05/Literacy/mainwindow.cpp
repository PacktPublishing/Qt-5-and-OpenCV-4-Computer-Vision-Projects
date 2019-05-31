#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSplitter>
#include <QDebug>

#include "mainwindow.h"
#include "screencapturer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , currentImage(nullptr)
    , tesseractAPI(nullptr)
{
    initUI();
}

MainWindow::~MainWindow()
{
    // Destroy used object and release memory
    if(tesseractAPI != nullptr) {
        tesseractAPI->End();
        delete tesseractAPI;
    }
}

void MainWindow::initUI()
{
    this->resize(800, 600);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");

    // setup toolbar
    fileToolBar = addToolBar("File");

    // main area
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    splitter->addWidget(imageView);

    editor = new QTextEdit(this);
    splitter->addWidget(editor);

    QList<int> sizes = {400, 400};
    splitter->setSizes(sizes);

    setCentralWidget(splitter);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Application Information will be here!");

    createActions();
}

void MainWindow::createActions()
{
    // create actions, add them to menus
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    saveImageAsAction = new QAction("Save &Image as", this);
    fileMenu->addAction(saveImageAsAction);
    saveTextAsAction = new QAction("Save &Text as", this);
    fileMenu->addAction(saveTextAsAction);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(exitAction);

    // add actions to toolbars
    fileToolBar->addAction(openAction);
    captureAction = new QAction("Capture Screen", this);
    fileToolBar->addAction(captureAction);
    ocrAction = new QAction("OCR", this);
    fileToolBar->addAction(ocrAction);
    detectAreaCheckBox = new QCheckBox("Detect Text Areas", this);
    fileToolBar->addWidget(detectAreaCheckBox);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveImageAsAction, SIGNAL(triggered(bool)), this, SLOT(saveImageAs()));
    connect(saveTextAsAction, SIGNAL(triggered(bool)), this, SLOT(saveTextAs()));
    connect(ocrAction, SIGNAL(triggered(bool)), this, SLOT(extractText()));
    connect(captureAction, SIGNAL(triggered(bool)), this, SLOT(captureScreen()));

    setupShortcuts();
}

void MainWindow::openImage()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList filePaths;
    if (dialog.exec()) {
        filePaths = dialog.selectedFiles();
        showImage(filePaths.at(0));
    }
}


void MainWindow::showImage(QPixmap image)
{
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
 }

void MainWindow::showImage(QString path)
{
    QPixmap image(path);
    showImage(image);
    currentImagePath = path;
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width())
        .arg(image.height()).arg(QFile(path).size());
    mainStatusLabel->setText(status);
}

void MainWindow::showImage(cv::Mat mat)
{
    QImage image(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(image);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
}

void MainWindow::saveImageAs()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Noting to save.");
        return;
    }
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Image As ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        if(QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0))) {
            currentImage->pixmap().save(fileNames.at(0));
        } else {
            QMessageBox::information(this, "Error", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::saveTextAs()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Text As ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Text files (*.txt)"));
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        if(QRegExp(".+\\.(txt)").exactMatch(fileNames.at(0))) {
            QFile file(fileNames.at(0));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::information(this, "Error", "Can't save text.");
                return;
            }
            QTextStream out(&file);
            out << editor->toPlainText() << "\n";
        } else {
            QMessageBox::information(this, "Error", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::setupShortcuts()
{
    QList<QKeySequence> shortcuts;
    shortcuts << (Qt::CTRL + Qt::Key_O);
    openAction->setShortcuts(shortcuts);

    shortcuts.clear();
    shortcuts << (Qt::CTRL + Qt::Key_Q);
    exitAction->setShortcuts(shortcuts);
}

void MainWindow::extractText()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No opened image.");
        return;
    }

    char *old_ctype = strdup(setlocale(LC_ALL, NULL));
    setlocale(LC_ALL, "C");
    if (tesseractAPI == nullptr) {
        tesseractAPI = new tesseract::TessBaseAPI();
        // Initialize tesseract-ocr with English, with specifying tessdata path
        if (tesseractAPI->Init(TESSDATA_PREFIX, "eng")) {
            QMessageBox::information(this, "Error", "Could not initialize tesseract.");
            return;
        }
    }

    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);

    tesseractAPI->SetImage(image.bits(), image.width(), image.height(),
        3, image.bytesPerLine());

    if (detectAreaCheckBox->checkState() == Qt::Checked) {
        std::vector<cv::Rect> areas;
        cv::Mat newImage = detectTextAreas(image, areas);
        showImage(newImage);
        editor->setPlainText("");
        for(cv::Rect &rect : areas) {
            tesseractAPI->SetRectangle(rect.x, rect.y, rect.width, rect.height);
            char *outText = tesseractAPI->GetUTF8Text();
            editor->setPlainText(editor->toPlainText() + outText);
            delete [] outText;
        }
    } else {
        char *outText = tesseractAPI->GetUTF8Text();
        editor->setPlainText(outText);
        delete [] outText;
    }

    setlocale(LC_ALL, old_ctype);
    free(old_ctype);
}

cv::Mat MainWindow::detectTextAreas(QImage &image, std::vector<cv::Rect> &areas)
{
    float confThreshold = 0.5;
    float nmsThreshold = 0.4;
    int inputWidth = 320;
    int inputHeight = 320;
    std::string model = "./frozen_east_text_detection.pb";
    // Load DNN network.
    if (net.empty()) {
        net = cv::dnn::readNet(model);
    }

    std::vector<cv::Mat> outs;
    std::vector<std::string> layerNames(2);
    layerNames[0] = "feature_fusion/Conv_7/Sigmoid";
    layerNames[1] = "feature_fusion/concat_3";

    cv::Mat frame = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,
        image.bits(),
        image.bytesPerLine()).clone();
    cv::Mat blob;

    cv::dnn::blobFromImage(
        frame, blob,
        1.0, cv::Size(inputWidth, inputHeight),
        cv::Scalar(123.68, 116.78, 103.94), true, false
    );
    net.setInput(blob);
    net.forward(outs, layerNames);

    cv::Mat scores = outs[0];
    cv::Mat geometry = outs[1];

    std::vector<cv::RotatedRect> boxes;
    std::vector<float> confidences;
    decode(scores, geometry, confThreshold, boxes, confidences);

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // Render detections.
    cv::Point2f ratio((float)frame.cols / inputWidth, (float)frame.rows / inputHeight);
    cv::Scalar green = cv::Scalar(0, 255, 0);

    for (size_t i = 0; i < indices.size(); ++i) {
        cv::RotatedRect& box = boxes[indices[i]];
        cv::Rect area = box.boundingRect();
        area.x *= ratio.x;
        area.width *= ratio.x;
        area.y *= ratio.y;
        area.height *= ratio.y;
        areas.push_back(area);
        cv::rectangle(frame, area, green, 1);
        QString index = QString("%1").arg(i);
        cv::putText(
            frame, index.toStdString(), cv::Point2f(area.x, area.y - 2),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, green, 1
        );
    }
    return frame;
}

void MainWindow::decode(const cv::Mat& scores, const cv::Mat& geometry, float scoreThresh,
    std::vector<cv::RotatedRect>& detections, std::vector<float>& confidences)
{
    CV_Assert(scores.dims == 4); CV_Assert(geometry.dims == 4);
    CV_Assert(scores.size[0] == 1); CV_Assert(scores.size[1] == 1);
    CV_Assert(geometry.size[0] == 1);  CV_Assert(geometry.size[1] == 5);
    CV_Assert(scores.size[2] == geometry.size[2]);
    CV_Assert(scores.size[3] == geometry.size[3]);

    detections.clear();
    const int height = scores.size[2];
    const int width = scores.size[3];
    for (int y = 0; y < height; ++y) {
        const float* scoresData = scores.ptr<float>(0, 0, y);
        const float* x0_data = geometry.ptr<float>(0, 0, y);
        const float* x1_data = geometry.ptr<float>(0, 1, y);
        const float* x2_data = geometry.ptr<float>(0, 2, y);
        const float* x3_data = geometry.ptr<float>(0, 3, y);
        const float* anglesData = geometry.ptr<float>(0, 4, y);
        for (int x = 0; x < width; ++x) {
            float score = scoresData[x];
            if (score < scoreThresh)
                continue;

            // Decode a prediction.
            // Multiple by 4 because feature maps are 4 time less than input image.
            float offsetX = x * 4.0f, offsetY = y * 4.0f;
            float angle = anglesData[x];
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            float h = x0_data[x] + x2_data[x];
            float w = x1_data[x] + x3_data[x];

            cv::Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x],
                offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
            cv::Point2f p1 = cv::Point2f(-sinA * h, -cosA * h) + offset;
            cv::Point2f p3 = cv::Point2f(-cosA * w, sinA * w) + offset;
            cv::RotatedRect r(0.5f * (p1 + p3), cv::Size2f(w, h), -angle * 180.0f / (float)CV_PI);
            detections.push_back(r);
            confidences.push_back(score);
        }
    }
}

void MainWindow::captureScreen()
{
    this->setWindowState(this->windowState() | Qt::WindowMinimized);
    QTimer::singleShot(500, this, SLOT(startCapture()));
}

void MainWindow::startCapture()
{
    ScreenCapturer *cap = new ScreenCapturer(this);
    cap->show();
    cap->activateWindow();
}

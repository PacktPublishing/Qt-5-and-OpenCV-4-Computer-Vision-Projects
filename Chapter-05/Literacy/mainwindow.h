#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QTextEdit>
#include <QCheckBox>
#include <QTimer>

#include "tesseract/baseapi.h"

#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();
    void showImage(QPixmap);

private:
    void initUI();
    void createActions();
    void showImage(QString);
    void showImage(cv::Mat);
    void setupShortcuts();

    void decode(const cv::Mat& scores, const cv::Mat& geometry, float scoreThresh,
        std::vector<cv::RotatedRect>& detections, std::vector<float>& confidences);
    cv::Mat detectTextAreas(QImage &image, std::vector<cv::Rect>&);

private slots:
    void openImage();
    void saveImageAs();
    void saveTextAs();
    void extractText();
    void captureScreen();
    void startCapture();

private:
    QMenu *fileMenu;

    QToolBar *fileToolBar;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QTextEdit *editor;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    QAction *openAction;
    QAction *saveImageAsAction;
    QAction *saveTextAsAction;
    QAction *exitAction;
    QAction *captureAction;
    QAction *ocrAction;
    QCheckBox *detectAreaCheckBox;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    tesseract::TessBaseAPI *tesseractAPI;
    cv::dnn::Net net;
};

#endif // MAINWINDOW_H

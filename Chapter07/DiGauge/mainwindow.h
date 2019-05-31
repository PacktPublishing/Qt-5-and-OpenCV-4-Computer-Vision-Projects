#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QListView>
#include <QCheckBox>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QMutex>
#include <QStandardItemModel>

#include "opencv2/opencv.hpp"

#include "capture_thread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private:
    void initUI();
    void createActions();
    void populateSavedList();

private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat*);
    void takePhoto();
    void appendSavedPhoto(QString name);
    void changeViewMode();

private:
    QMenu *fileMenu;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *exitAction;

    QMenu *viewMenu;

    QAction *birdEyeAction;
    QAction *eyeLevelAction;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QPushButton *shutterButton;

    QListView *saved_list;
    QStandardItemModel *list_model;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    cv::Mat currentFrame;

    // for capture thread
    QMutex *data_lock;
    CaptureThread *capturer;
};

#endif // MAINWINDOW_H

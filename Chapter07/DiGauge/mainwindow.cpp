#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QCameraInfo>
#include <QGridLayout>
#include <QIcon>
#include <QStandardItem>
#include <QSize>

#include "opencv2/videoio.hpp"

#include "mainwindow.h"
#include "utilities.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , fileMenu(nullptr)
    , capturer(nullptr)
{
    initUI();
    data_lock = new QMutex();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(1000, 800);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");
    viewMenu = menuBar()->addMenu("&View");

    // main area
    QGridLayout *main_layout = new QGridLayout();

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    main_layout->addWidget(imageView, 0, 0, 12, 1);

    // tools
    QGridLayout *tools_layout = new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);

    shutterButton = new QPushButton(this);
    shutterButton->setText("Take a Photo");
    tools_layout->addWidget(shutterButton, 0, 0, Qt::AlignHCenter);
    connect(shutterButton, SIGNAL(clicked(bool)), this, SLOT(takePhoto()));

    // list of saved photos
    saved_list = new QListView(this);
    saved_list->setViewMode(QListView::IconMode);
    saved_list->setResizeMode(QListView::Adjust);
    saved_list->setSpacing(5);
    saved_list->setWrapping(false);
    list_model = new QStandardItemModel(this);
    saved_list->setModel(list_model);
    main_layout->addWidget(saved_list, 13, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("DiGauge is Ready");

    createActions();
    populateSavedList();
}

void MainWindow::createActions()
{
    // create actions, add them to menus
    cameraInfoAction = new QAction("Camera &Information", this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction("&Open Camera", this);
    fileMenu->addAction(openCameraAction);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(exitAction);

    birdEyeAction = new QAction("Bird Eye View");
    birdEyeAction->setCheckable(true);
    viewMenu->addAction(birdEyeAction);
    eyeLevelAction = new QAction("Eye Level View");
    eyeLevelAction->setCheckable(true);
    viewMenu->addAction(eyeLevelAction);

    birdEyeAction->setChecked(true);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));

    connect(birdEyeAction, SIGNAL(triggered(bool)), this, SLOT(changeViewMode()));
    connect(eyeLevelAction, SIGNAL(triggered(bool)), this, SLOT(changeViewMode()));
}

void MainWindow::showCameraInfo()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Cameras: \n");

    foreach (const QCameraInfo &cameraInfo, cameras)
        info += "  - " + cameraInfo.deviceName() + "\n";

    QMessageBox::information(this, "Cameras", info);
}


void MainWindow::openCamera()
{
    if(capturer != nullptr) {
        // if a thread is already running, stop it
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }
    // I am using my second camera whose Index is 2.  Usually, the
    // Index of the first camera is 0.
    int camID = 2;
    capturer = new CaptureThread(camID, data_lock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
    capturer->start();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));

    birdEyeAction->setChecked(true);
    eyeLevelAction->setChecked(false);
}


void MainWindow::updateFrame(cv::Mat *mat)
{
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    QImage frame(
        currentFrame.data,
        currentFrame.cols,
        currentFrame.rows,
        currentFrame.step,
        QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);

    imageScene->clear();
    imageView->resetMatrix();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}


void MainWindow::takePhoto()
{
    if(capturer != nullptr) {
        capturer->takePhoto();
    }
}


void MainWindow::populateSavedList()
{
    QDir dir(Utilities::getDataPath());
    QStringList nameFilters;
    nameFilters << "*.jpg";
    QFileInfoList files = dir.entryInfoList(
        nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    foreach(QFileInfo photo, files) {
        QString name = photo.baseName();
        QStandardItem *item = new QStandardItem();
        list_model->appendRow(item);
        QModelIndex index = list_model->indexFromItem(item);
        list_model->setData(index, QPixmap(photo.absoluteFilePath()).scaledToHeight(145), Qt::DecorationRole);
        list_model->setData(index, name, Qt::DisplayRole);
    }
}

void MainWindow::appendSavedPhoto(QString name)
{
    QString photo_path = Utilities::getPhotoPath(name, "jpg");
    QStandardItem *item = new QStandardItem();
    list_model->appendRow(item);
    QModelIndex index = list_model->indexFromItem(item);
    list_model->setData(index, QPixmap(photo_path).scaledToHeight(145), Qt::DecorationRole);
    list_model->setData(index, name, Qt::DisplayRole);
    saved_list->scrollTo(index);
}

void MainWindow::changeViewMode()
{
    CaptureThread::ViewMode mode = CaptureThread::BIRDEYE;
    QAction *active_action = qobject_cast<QAction*>(sender());
    if(active_action == birdEyeAction) {
        birdEyeAction->setChecked(true);
        eyeLevelAction->setChecked(false);
        mode = CaptureThread::BIRDEYE;
    } else if (active_action == eyeLevelAction) {
        eyeLevelAction->setChecked(true);
        birdEyeAction->setChecked(false);
        mode = CaptureThread::EYELEVEL;
    }
    if(capturer != nullptr) {
        capturer->setViewMode(mode);
    }
}

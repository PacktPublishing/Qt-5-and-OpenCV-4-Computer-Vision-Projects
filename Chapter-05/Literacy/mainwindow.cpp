#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QSplitter>
#include <QDebug>

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , currentImage(nullptr)
    , tesseractAPI(nullptr)
{
    initUI();
}

MainWindow::~MainWindow()
{
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
    ocrAction = new QAction("OCR", this);
    fileToolBar->addAction(ocrAction);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveImageAsAction, SIGNAL(triggered(bool)), this, SLOT(saveImageAs()));
    connect(saveTextAsAction, SIGNAL(triggered(bool)), this, SLOT(saveTextAs()));
    connect(ocrAction, SIGNAL(triggered(bool)), this, SLOT(extractText()));

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


void MainWindow::showImage(QString path)
{
    imageScene->clear();
    imageView->resetMatrix();
    QPixmap image(path);
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width())
        .arg(image.height()).arg(QFile(path).size());
    mainStatusLabel->setText(status);
    currentImagePath = path;
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
    tesseractAPI = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, with specifying tessdata path
    if (tesseractAPI->Init(TESSDATA_PREFIX, "eng")) {
        QMessageBox::information(this, "Error", "Could not initialize tesseract.");
        return;
    }

    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);

    tesseractAPI->SetImage(image.bits(), image.width(), image.height(),
        3, image.bytesPerLine());
    char *outText = tesseractAPI->GetUTF8Text();
    editor->setPlainText(outText);
    // Destroy used object and release memory
    tesseractAPI->End();
    delete tesseractAPI;
    tesseractAPI = nullptr;
    delete [] outText;
    setlocale(LC_ALL, old_ctype);
    free(old_ctype);
}

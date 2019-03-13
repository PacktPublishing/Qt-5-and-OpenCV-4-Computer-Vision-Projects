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
#include <QTextEdit>

#include "tesseract/baseapi.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private:
    void initUI();
    void createActions();
    void showImage(QString);
    void setupShortcuts();

private slots:
    void openImage();
    void saveImageAs();
    void saveTextAs();
    void extractText();

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
    QAction *ocrAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    tesseract::TessBaseAPI *tesseractAPI;
};

#endif // MAINWINDOW_H

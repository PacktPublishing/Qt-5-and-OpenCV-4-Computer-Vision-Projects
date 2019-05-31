#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>

#include "mainwindow.h"

class ScreenCapturer : public QWidget {
    Q_OBJECT

public:
    explicit ScreenCapturer(MainWindow *w);
    ~ScreenCapturer();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void closeMe();
    void confirmCapture();

private:
    void initShortcuts();
    QPixmap captureDesktop();

private:
    MainWindow *window;
    QPixmap screen;
    QPoint p1, p2;
    bool mouseDown;
};

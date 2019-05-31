#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QRect>
#include <QScreen>
#include <QPainter>
#include <QColor>
#include <QRegion>
#include <QShortcut>

#include "screencapturer.h"

ScreenCapturer::ScreenCapturer(MainWindow *w):
    QWidget(nullptr), window(w)
{
    setWindowFlags(
        Qt::BypassWindowManagerHint
        | Qt::WindowStaysOnTopHint
        | Qt::FramelessWindowHint
        | Qt::Tool
    );

    setAttribute(Qt::WA_DeleteOnClose);
    // setMouseTracking(true);

    screen = captureDesktop();
    resize(screen.size());
    initShortcuts();
}

ScreenCapturer::~ScreenCapturer() {
}

QPixmap ScreenCapturer::captureDesktop() {
    QRect geometry;
    for (QScreen *const screen : QGuiApplication::screens()) {
        geometry = geometry.united(screen->geometry());
    }

    QPixmap pixmap(QApplication::primaryScreen()->grabWindow(
                  QApplication::desktop()->winId(),
                  geometry.x(),
                  geometry.y(),
                  geometry.width(),
                  geometry.height()
        ));
    pixmap.setDevicePixelRatio(QApplication::desktop()->devicePixelRatio());
    return pixmap;
}

void ScreenCapturer::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, screen);

    QRegion grey(rect());
    if(p1.x() != p2.x() && p1.y() != p2.y()) {
        painter.setPen(QColor(200, 100, 50, 255));
        painter.drawRect(QRect(p1, p2));
        grey = grey.subtracted(QRect(p1, p2));
    }
    painter.setClipRegion(grey);
    QColor overlayColor(20, 20, 20, 50);
    painter.fillRect(rect(), overlayColor);
    painter.setClipRect(rect());
}

void ScreenCapturer::mousePressEvent(QMouseEvent *event)
{
    mouseDown = true;
    p1 = event->pos();
    p2 = event->pos();
    update();
}

void ScreenCapturer::mouseMoveEvent(QMouseEvent *event)
{
    if(!mouseDown) return;
    p2 = event->pos();
    update();
}

void ScreenCapturer::mouseReleaseEvent(QMouseEvent *event)
{
    mouseDown = false;
    p2 = event->pos();
    update();
}

void ScreenCapturer::closeMe()
{
    this->close();
    window->showNormal();
    window->activateWindow();
}

void ScreenCapturer::confirmCapture()
{
    QPixmap image = screen.copy(QRect(p1, p2));
    window->showImage(image);
    closeMe();
}

void ScreenCapturer::initShortcuts() {
    new QShortcut(Qt::Key_Escape, this, SLOT(closeMe()));
    new QShortcut(Qt::Key_Return, this, SLOT(confirmCapture()));
}

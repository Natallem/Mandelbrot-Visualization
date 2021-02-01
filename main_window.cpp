#include "main_window.h"

#include "ui_main_window.h"
#include <QGridLayout>
#include <QPainter>
#include <QPushButton>
#include <complex>
#include <iostream>
#include <QMouseEvent>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::main_window), drag_pos(-1, -1), center_offset(0, 0) {
    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
    print(center_offset, "setup");
}

main_window::~main_window() = default;

void main_window::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - drag_pos;
        center_offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = event->pos();
        print("before update in mouseMoveEvent");
        update();
    }
}

void main_window::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        assert(drag_pos == QPoint(-1, -1));
        drag_pos = event->pos();
        print("mousePressEvent");

    }
}


void main_window::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint diff = event->pos() - drag_pos;
        center_offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = QPoint(-1, -1);
        print("before update in mouseReleaseEvent");
        update();
    }

}

void main_window::paintEvent(QPaintEvent *ev) {
    auto t = center_offset;
    print(t, "printEvent");
    QMainWindow::paintEvent(ev);
    int dev = 1;
    int h = height() / dev;
    int w = width() / dev;
    QImage img(w, h, QImage::Format_RGB888);
    unsigned char *data = img.bits();
    size_t stride = img.bytesPerLine();
    for (int y = 0; y != h; ++y) {
        unsigned char *p = data + y * stride;
        for (int x = 0; x != w; ++x) {
            *p++ = value(x, y, w, h);
            *p++ = 0;
            *p++ = 0;
        }
    }
    QPainter p(this);

    QImage img2 = img.scaled(w * dev, h * dev, Qt::KeepAspectRatio);
    p.drawImage(0, 0, img2);
}



size_t main_window::value(int x, int y, int w, int h) const {
    if (x == w / 2 || y == h / 2) {
        return 255;
    }

    std::complex<double> c(x - w / 2, y - h / 2);
    c += center_offset;
    c *= scale;
    if (c.imag() == -1.)
        return 255;

    std::complex<double> z = 0;
    size_t const MAX_STEP = 255;
    for (size_t i = 1; i <= MAX_STEP; i++) {
        if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
            return i;
        }
        z = z * z + c;
    }
    return 0;
}

template<class T>
void main_window::print(T &t, std::string str) {
    std::cout << str << " " << t << "\n";
    std::cout.flush();
}

QImage main_window::getImage(int x, int y) {
    int edge = 1024;


    return QImage();
}


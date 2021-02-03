#include "main_window.h"

#include "ui_main_window.h"
#include <QGridLayout>
#include <QPainter>
#include <QPushButton>
#include <complex>
#include <iostream>
#include <QMouseEvent>
#include <algorithm>
#include <QThread>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::main_window),
          drag_pos(-1, -1),
          center_offset(0, 0),
          cache(image_cache(32, scale, 1/*std::max(1, QThread::idealThreadCount())*/)) {
    print("hello");
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
        update();
    }

}

void main_window::paintEvent(QPaintEvent *ev) {
    static bool tot = true;

    print(height(), "h");
    print(width(), "w");
    auto t = center_offset;
    print(t, "printEvent");
    QMainWindow::paintEvent(ev);
    int dev = 1;
    int h = height() / dev;
    int w = width() / dev;
    std::complex<double> center(w / 2, h / 2);
    complex right_upper_corner(0, 0);
    right_upper_corner += center_offset - center;
    std::complex<int> shift(floor(right_upper_corner.real() / sub_image_size) * sub_image_size,
                            floor(right_upper_corner.imag() / sub_image_size) * sub_image_size);
    shift -= right_upper_corner;
    QPainter p(this);
    std::queue<sub_image *> to_draw;
    bool stop = false;
    if (tot){
        print("size "+ std::to_string(w + sub_image_size)+ " " + std::to_string(h + sub_image_size));
    }
    size_t iter = 0;
    while (!stop) {
        iter++;
        stop = true;
        for (int y = 0; y <= h + sub_image_size; y += sub_image_size) {
            for (int x = 0; x <= w + sub_image_size; x += sub_image_size) {
                complex d(x, y);
                d += shift;
//                if (tot) print(d, "iter " + std::to_string(iter) + " ");
//                complex c(x + shift.real(), y + shift.imag());
                complex c(d + center_offset - center);
                sub_image *img = cache.get_sub_image(c.real(), c.imag());
                int size = img->get_width();
                if (size != sub_image_size) {
                    stop = false;
//                    print(аsize);
//                    to_draw.push(img);
                }
                if (tot) print(d, "iter2 " + std::to_string(iter) + " ");
                QImage img2 = img->getQImage();
                if (tot) print(d, "iter3 " + std::to_string(iter) + " ");

                p.drawImage(x + shift.real(), y + shift.imag(),
                            (img2).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
                if (tot) print(d, "iter3 " + std::to_string(iter) + " ");
            }
        }
    }
    tot = false;
    print(cache.get_size(), "cache size");
/*
    while (!to_draw.empty()) {
        sub_image *img = to_draw.front();
        to_draw.pop();
        auto[vertex, _] = img->get_data();
        int size = img->get_width();
        if (size != sub_image_size) {
            to_draw.push(img);
        }
        p.drawImage(vertex.real() + shift.real(), vertex.imag() + shift.imag(),
                    (img->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
    }*/
}


size_t main_window::value(complex point) const {
//    if (point.imag() == -1.)
//        return 255;

    std::complex<double> z(0, 0);
    size_t const MAX_STEP = 255;
    for (size_t i = 1; i <= MAX_STEP; i++) {
        if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
            return i;
        }
        z = z * z + point;
    }
    return 0;
}

template<class T>
void main_window::print(T &&t, std::string str) {
    std::cout << str << " " << t << "\n";
    std::cout.flush();
}

QImage main_window::getImage(const complex &c) {
    assert(floor(abs(c.real())) == abs(c.real()));
    assert(floor(abs(c.imag())) == abs(c.imag()));
    assert(int(c.real()) % sub_image_size == 0);
    assert(int(c.imag()) % sub_image_size == 0);
    int h = sub_image_size;
    int w = sub_image_size;
    QImage img(w, h, QImage::Format_RGB888);
    size_t stride = img.bytesPerLine();
    unsigned char *data = img.bits();
    for (int y = 0; y != h; ++y) {
        unsigned char *p = data + y * stride;
        for (int x = 0; x != w; ++x) {
            *p++ = value((c + complex(x, y)) * scale);
            *p++ = 0;
            *p++ = 0;
        }
    }
    return img;
}

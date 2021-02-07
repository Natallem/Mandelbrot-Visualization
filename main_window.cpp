#include "main_window.h"

#include "ui_main_window.h"
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QThread>
#include <algorithm>
#include <complex>
#include <iostream>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent),
          ui(new Ui::main_window),
          offset(0, 0),
          cache(
                  image_cache(
                          sub_image_size,
                          initial_scale,
                          std::max(1, QThread::idealThreadCount())
                  )) {
    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
    center = -complex(width() / 2, height() / 2);
}

main_window::~main_window() = default;

void main_window::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_pressed = true;
        drag_pos = event->pos();
    }
}

void main_window::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && is_pressed) {
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = event->pos();
        update();
    }
}

void main_window::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_pressed = false;
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        update();
    }
}

void main_window::paintEvent(QPaintEvent *ev) {
    QMainWindow::paintEvent(ev);
    QPainter p(this);
    complex shift(floor((double) offset.real() / sub_image_size) * sub_image_size,
                  floor((double) offset.imag() / sub_image_size) * sub_image_size);
    shift -= offset;
    std::queue<std::pair<complex, sub_image *>> not_ready;
    for (int y = 0; y <= height() + sub_image_size; y += sub_image_size) {
        for (int x = 0; x <= width() + sub_image_size; x += sub_image_size) {
            complex d(x, y);
            d += shift;
            double scale;
            {
                std::lock_guard l(cache.worker.queue.m);
                scale = cache.worker.queue.scale;
            }
            sub_image *img = cache.get_sub_image((d + offset) * scale + center * initial_scale);
            int size = img->get_width();
            if (size != sub_image_size) {
                not_ready.push({d, img});
                continue;
            }
            {
                std::lock_guard lock(img->m);
                p.drawImage(d.real(), d.imag(),
                            (img->getImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            }
        }
    }
    bool retry = false;
    while (!not_ready.empty()) {
        auto &pair = not_ready.front();
        not_ready.pop();
        int size;
        {
            std::lock_guard lock(pair.second->m);
            size = pair.second->image.width();
            if (size != 1) {
                p.drawImage(pair.first.real(), pair.first.imag(),
                            (pair.second->getImage())
                                    .scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            }
        }
        if (size != sub_image_size) {
            if (size == 1) {
                not_ready.push(pair);
            }
            retry = true;
        }
    }
    if (retry) {
        update();
    }
}

template<class T>
void main_window::print(T &&t, std::string str) {
    std::cout << str << " " << t << "\n";
    std::cout.flush();
}

void main_window::resizeEvent(QResizeEvent *event) {
    update();
}

void main_window::wheelEvent(QWheelEvent *event) {
    double mul = -1;
    if (event->angleDelta().y() < 0) { mul = 1; }
    double scale_mul = 0.1;
    scale_mul = 1 + mul * scale_mul;
    double prev_scale = cache.worker.queue.get_scale();
    cache.change_scale(scale_mul);
    double new_scale = cache.worker.queue.get_scale();
    std::complex<double> x(event->position().x(), event->position().y());
    center = ((x + offset) * prev_scale + center * initial_scale - new_scale * x) / initial_scale;
    offset = complex(0, 0);
    update();
}




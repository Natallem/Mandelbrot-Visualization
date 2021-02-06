#include "main_window.h"

#include "ui_main_window.h"
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QThread>
#include <algorithm>
#include <complex>
#include <iostream>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::main_window), drag_pos(-1, -1), offset(0, 0),
          cache(image_cache(
                  sub_image_size,
                  initial_scale,
                  std::max(1, QThread::idealThreadCount())
          )) {
    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
    center = -complex(width() / 2, height() / 2);
}

main_window::~main_window() = default;

void main_window::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && is_pressed) {
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = event->pos();
        update();
    }
}

void main_window::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_pressed = true;
        assert(drag_pos == QPoint(-1, -1));
        drag_pos = event->pos();
    }
}

void main_window::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_pressed = false;
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = QPoint(-1, -1);
        update();
    }
}

void main_window::paintEvent(QPaintEvent *ev) {
    QMainWindow::paintEvent(ev);
    QPainter p(this);
    int h = height();
    int w = width();
    complex right_upper_corner = offset;
    complex shift(floor((double) right_upper_corner.real() / sub_image_size) * sub_image_size,
                  floor((double) right_upper_corner.imag() / sub_image_size) * sub_image_size);
    shift -= right_upper_corner;
    std::queue<std::pair<complex, sub_image *>> not_ready;
    for (int y = 0; y <= h + sub_image_size; y += sub_image_size) {
        for (int x = 0; x <= w + sub_image_size; x += sub_image_size) {

//            QApplication::processEvents(QEventLoop::AllEvents);
            complex d(x, y);
            d += shift;
            double scale;
            {
                std::lock_guard l(cache.worker.queue.m);
                scale = cache.worker.queue.scale;
            }
            complex c(d + offset);
            std::complex<double> cc(c.real() * scale, c.imag() * scale);
            cc += std::complex<double>(center * initial_scale);
            sub_image *img = cache.get_sub_image(cc.real(), cc.imag());
            int size = img->get_width();
            if (size != sub_image_size) {
                not_ready.push({d, img});
                continue;
            }
            {
                std::lock_guard lock(img->m);
                p.drawImage(d.real(), d.imag(),
                            (img->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
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
                            (pair.second->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            }
        }

        if (size != sub_image_size){
            if (size == 1){
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
//
//void main_window::resizeEvent(QResizeEvent *event) {
//    update();
//}

void main_window::wheelEvent(QWheelEvent *event) {
    double mul = -1;
    if (event->angleDelta().y() < 0)
        mul = 1;
    double alpha1 = 0.1;
    alpha1 = 1 + mul * alpha1;
    double alpha = initial_scale;
    double betta = cache.worker.queue.get_scale();
    cache.change_scale(alpha1);
    double gamma = cache.worker.queue.get_scale();
    std::complex<double> x(event->position().x(), event->position().y());
    center = ((x + offset) * betta + center * alpha - gamma * x) / alpha;
    offset = complex(0, 0);
    update();
}




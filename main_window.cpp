#include "main_window.h"

#include <QMouseEvent>
#include <QPainter>
#include <QThread>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::main_window), prev_width(0), prev_height(0), offset(0, 0), cache(
        image_cache(
                sub_image_degree,
                initial_scale,
                std::max(1, QThread::idealThreadCount()))) {
    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
    center = -complex(width() / 2, height() / 2);
    check_size_of_sub_images(true);
}

void main_window::paintEvent(QPaintEvent *ev) {
    QMainWindow::paintEvent(ev);
    QPainter p(this);
    int sub_image_size = (1 << sub_image_degree);
    complex shift(floor((precise_t) offset.real() / sub_image_size) * sub_image_size,
                  floor((precise_t) offset.imag() / sub_image_size) * sub_image_size);
    shift -= offset;
    double scale = cache.get_cur_scale();
    std::queue<std::pair<complex, sub_image *>> not_ready;
    bool need_retry = false;
    for (int y = 0; y <= height() + sub_image_size; y += sub_image_size) {
        for (int x = 0; x <= width() + sub_image_size; x += sub_image_size) {
            complex d(x, y);
            d += shift;
            sub_image *img = cache.get_sub_image((d + offset) * scale + center * initial_scale, sub_image_degree);
            int size = img->get_size();
            if (!need_retry && size != sub_image_size) {
                need_retry = true;
            }
            if (size == 1) {
                not_ready.push({d, img});
                continue;
            }
            auto ratio= (double) sub_image_size / size;
            p.setTransform(QTransform(ratio, 0, 0, ratio, d.real(), d.imag()));
            p.drawImage(0, 0, img->get_ready_image());
        }
    }
    while (!not_ready.empty()) {
        auto[d, img] = not_ready.front();
        not_ready.pop();
        auto size = img->get_size();
        if (size == 1) {
            not_ready.push({d, img});
            continue;
        }
        auto ratio = (double) sub_image_size / size;
        p.setTransform(QTransform(ratio, 0, 0, ratio, d.real(), d.imag()));
        p.drawImage(0, 0, img->get_ready_image());
    }
    if (need_retry) {
        update();
    }
}

void main_window::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_pressed = true;
        drag_pos = event->pos();
    }
}

void main_window::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && is_pressed) {
        QPoint diff = event->pos() - drag_pos;
        offset -= complex(diff.x(), diff.y());
        drag_pos = event->pos();
        update();
    }
}

void main_window::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_pressed = false;
        QPoint diff = event->pos() - drag_pos;
        offset -= complex(diff.x(), diff.y());
        update();
    }
}

void main_window::wheelEvent(QWheelEvent *event) {
    double mul = -1;
    if (event->angleDelta().y() < 0) {
        mul = 1;
    }
    double scale_mul = 0.1;
    scale_mul = 1 + mul * scale_mul;
    double prev_scale = cache.get_cur_scale();
    cache.change_scale(scale_mul);
    check_size_of_sub_images(true);
    double new_scale = cache.get_cur_scale();
    complex x(event->position().x(), event->position().y());
    center = ((x + offset) * prev_scale + center * initial_scale - new_scale * x) / initial_scale;
    offset = complex(0, 0);
    update();
}

void main_window::resizeEvent(QResizeEvent *event) {
    check_size_of_sub_images();
    update();
}

void main_window::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMaximized()) {
            check_size_of_sub_images();
        }
    }
}

void main_window::check_size_of_sub_images(bool forced) {
    if (width() == prev_width && height() == prev_height)
        return;
    int min_sub_image_size = std::max(width(), height()) / granularity;
    int new_size = 1;
    int new_degree = 0;
    while (new_size < min_sub_image_size) {
        new_size <<= 1;
        ++new_degree;
    }
    if (new_size == (1 << sub_image_degree))
        return;
    if (forced) {
        accept_sub_image_resize(new_degree);
        return;
    }
    int mul = 4;
    if (new_size > (1 << sub_image_degree) * mul // if new_size much greater then previous one
        || new_size * mul < (1 << sub_image_degree)) { // if new_size much smaller then previous one
        accept_sub_image_resize(new_degree);
    }
}

void main_window::accept_sub_image_resize(int new_degree) {
    sub_image_degree = new_degree;
    cache.change_sub_image_degree(new_degree);
    prev_height = height();
    prev_width = width();
}

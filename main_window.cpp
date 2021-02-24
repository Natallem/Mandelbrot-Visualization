#include "main_window.h"

#include "ui_main_window.h"
#include <QMouseEvent>
#include <QPainter>
#include <QThread>
#include <algorithm>
#include <complex>
#include <iostream>

main_window::main_window(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
    , prev_width(0)
    , prev_height(0)
    , offset(0, 0)
    , cache(
          image_cache(
              sub_image_size,
              initial_scale,
              std::max(1, QThread::idealThreadCount())))
{
    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
    center = -complex(width() / 2, height() / 2);
    check_size_of_sub_images(true);
}

main_window::~main_window() = default;

void main_window::paintEvent(QPaintEvent* ev)
{
    QPainter p(this);
    print("paintEvent retry");
    QMainWindow::paintEvent(ev);
    complex shift(floor((double)offset.real() / sub_image_size) * sub_image_size,
        floor((double)offset.imag() / sub_image_size) * sub_image_size);
    shift -= offset;
    std::vector<std::pair<complex, sub_image*>> not_ready;
    bool should_retry = false;
    for (int y = 0; y <= height() + sub_image_size; y += sub_image_size) {
        for (int x = 0; x <= width() + sub_image_size; x += sub_image_size) {
            complex d(x, y);
            d += shift;
            double scale = cache.get_cur_scale();
            sub_image* img = cache.get_sub_image((d + offset) * scale + center * initial_scale);
            int size = img->get_width();
            if (size == 1) {
                not_ready.emplace_back( d, img );
                continue;
            }
            if (size != sub_image_size)
                should_retry = true;
            auto ratiow = (double)sub_image_size / size;
            //            auto ratioh = ratiow;
            p.setTransform(
                QTransform(
                    ratiow, 0,
                    0, ratiow,
                    d.real(),
                    d.imag()));
            {
                std::lock_guard lock(img->m);
                p.drawImage(0, 0, img->getImage());
            }
            /*{
                std::lock_guard lock(img->m);
                p.drawImage(d.real(), d.imag(),
                    (img->getImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            }*/
        }
    }

    for (auto& pair : not_ready) {
        sub_image * img = pair.second;
        complex d = pair.first;
        int size = img->get_width();
        auto ratiow = (double)sub_image_size / size;
        p.setTransform(
                QTransform(
                        ratiow, 0,
                        0, ratiow,
                        d.real(),
                        d.imag()));
        {
            std::lock_guard lock(img->m);
            p.drawImage(0, 0, img->getImage());
        }
        if (size != sub_image_size)
            should_retry = true;
      /*  {
            std::lock_guard lock(pair.second->m);
            size = pair.second->image.width();

            //            if (size != 1) {
            p.drawImage(pair.first.real(), pair.first.imag(),
                (pair.second->getImage())
                    .scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            //            }
        }
        if (size != sub_image_size) {
            //            if (size == 1) {
            //            not_ready.push(pair);
            //            }
            should_retry = true;
        }*/
    } /*
    while (!not_ready.empty()) {
        std::pair<complex, sub_image*> pair = not_ready.front();
        not_ready.pop();
        int size;
        {
            std::lock_guard lock(pair.second->m);
            size = pair.second->image.width();
            //            if (size != 1) {
            p.drawImage(pair.first.real(), pair.first.imag(),
                (pair.second->getImage())
                    .scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            //            }
        }
        if (size != sub_image_size) {
            //            if (size == 1) {
            //            not_ready.push(pair);
            //            }
            should_retry = true;
        }
    }*/
    if (should_retry) {
        update();
    }
}

void main_window::mousePressEvent(QMouseEvent* event)
{
    print("mousePressEvent");
    if (event->button() == Qt::LeftButton) {
        is_pressed = true;
        drag_pos = event->pos();
    }
}

void main_window::mouseMoveEvent(QMouseEvent* event)
{
    print("mouseMoveEvent");
    if (event->buttons() & Qt::LeftButton && is_pressed) {
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = event->pos();
        update();
    }
}

void main_window::mouseReleaseEvent(QMouseEvent* event)
{
    print("changeEvent");
    if (event->button() == Qt::LeftButton) {
        is_pressed = false;
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        update();
    }
}

void main_window::wheelEvent(QWheelEvent* event)
{
    print("wheelEvent");
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
    std::complex<double> x(event->position().x(), event->position().y());
    center = ((x + offset) * prev_scale + center * initial_scale - new_scale * x) / initial_scale;
    offset = complex(0, 0);
    update();
}

void main_window::resizeEvent(QResizeEvent* event)
{
    print("changeEvent");
    check_size_of_sub_images();
    update();
}

void main_window::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMaximized()) {
            print("changeEvent");
            check_size_of_sub_images();
        }
    }
}

void main_window::print(std::string s)
{
    std::cout << s << "\n";
    std::cout.flush();
}

void main_window::check_size_of_sub_images(bool forced)
{
    if (width() == prev_width && height() == prev_height)
        return;
    int min_sub_image_size = std::max(width(), height()) / detalization;
    int new_size = 1;
    while (new_size < min_sub_image_size) {
        new_size <<= 1;
    }
    if (new_size == sub_image_size)
        return;
    if (forced) {
        accept_sub_image_resize(new_size);
        return;
    }
    int mul = 4;
    if (new_size > sub_image_size * mul // if new_size much greater then previous one
        || new_size * mul < sub_image_size) { // if new_size much smaller then previous one
        accept_sub_image_resize(new_size);
    }
}

void main_window::accept_sub_image_resize(int new_size)
{
    sub_image_size = new_size;
    cache.change_sub_image_size(new_size);
    prev_height = height();
    prev_width = width();
}

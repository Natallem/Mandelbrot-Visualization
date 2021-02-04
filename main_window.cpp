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

main_window::main_window(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
    , drag_pos(-1, -1)
    , center_offset(0, 0)
    , cache(image_cache(sub_image_size, scale, std::max(1, QThread::idealThreadCount() - 1)))
{
    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
}

main_window::~main_window() = default;

void main_window::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton && is_pressed) {
        print("mouseMoveEvent");
        QPoint diff = event->pos() - drag_pos;
        center_offset -= std::complex<int>(diff.x(), diff.y());
        drag_pos = event->pos();
        update();
    }
}

void main_window::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        is_pressed = true;
        print("mousePressEvent");
        assert(drag_pos == QPoint(-1, -1));
        drag_pos = event->pos();
    }
}

void main_window::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        is_pressed = false;
        print("mouseReleaseEvent");
        QPoint diff = event->pos() - drag_pos;
        center_offset -= std::complex<int>(diff.x(), diff.y());
        drag_pos = QPoint(-1, -1);
        update();
    }
}

void main_window::resizeEvent(QResizeEvent* event)
{
    update();
    print(center_offset);
    event->accept();
}

void main_window::paintEvent(QPaintEvent* ev)
{
    QMainWindow::paintEvent(ev);
    QPainter p(this);
    //    QImage image(1,1, QImage::Format_RGB888);
    //    image.setPixel(0,0,0);
    //    p.drawImage(0,0,image.scaled(width(), height(), Qt::KeepAspectRatio));

    int dev = 1;
    int h = height() / dev;
    int w = width() / dev;
    std::complex<int> center(w / 2, h / 2);
    std::complex<int> right_upper_corner(0, 0);
    right_upper_corner += center_offset - center;
    std::complex<int> shift(floor((double)right_upper_corner.real() / sub_image_size) * sub_image_size,
        floor((double)right_upper_corner.imag() / sub_image_size) * sub_image_size);
    shift -= right_upper_corner;
    std::queue<sub_image*> to_draw;
    bool stop = false;
    size_t iter = 0;
    while (!stop) {
        iter++;
        stop = true;
        for (int y = 0; y <= h + sub_image_size; y += sub_image_size) {
            for (int x = 0; x <= w + sub_image_size; x += sub_image_size) {
                std::complex<int> d(x, y);
                d += shift;
                std::complex<int> c(d + center_offset - center);
                sub_image* img = cache.get_sub_image(c.real(), c.imag());
                int size = img->get_width();
                if (size != sub_image_size) {
                    //                    to_draw.push(img);
                    stop = false;
                }
                {
                    std::lock_guard gr(img->m);
                    p.drawImage(x + shift.real(), y + shift.imag(),
                        (img->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
                }
            }
        }
    }

    while (false && !to_draw.empty()) {
        sub_image* img = to_draw.front();
        to_draw.pop();
        auto [vertex, _] = img->get_data();
        vertex -= center_offset - center;
        int size = img->get_width();
        if (size != sub_image_size) {
            to_draw.push(img);
        }
        p.drawImage(vertex.real() + shift.real(), vertex.imag() + shift.imag(),
            (img->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
    }
}

template <class T>
void main_window::print(T&& t, std::string str)
{
    std::cout << str << " " << t << "\n";
    std::cout.flush();
}

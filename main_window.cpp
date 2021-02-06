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
          cache(image_cache(sub_image_size, initial_scale,
//            1
                            std::max(1, QThread::idealThreadCount())


          )) {

    ui->setupUi(this);
    this->setWindowTitle("Mandelbrot");
    center = -complex(width() / 2, height() / 2);

}

main_window::~main_window() = default;

void main_window::mouseMoveEvent(QMouseEvent *event) {

    if (event->buttons() & Qt::LeftButton && is_pressed) {
//        print("mouseMoveEvent");

        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = event->pos();
//        print(center, "center");
//        print(center_offset, "center_offset");
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
//    print("mouseReleaseEvent");

    if (event->button() == Qt::LeftButton) {
        is_pressed = false;
        QPoint diff = event->pos() - drag_pos;
        offset -= std::complex<double>(diff.x(), diff.y());
        drag_pos = QPoint(-1, -1);
        update();
    }
}

void main_window::paintEvent(QPaintEvent *ev) {
    //    int count1 = 0;
    //    int others = 0;
    //    print("paintEvent");
    //        static int time_for_one_print = 1;
    //        static int print_time = 1;

    QMainWindow::paintEvent(ev);
    QPainter p(this);
    /*   QImage imag(1,1, QImage::Format_RGB888);
       imag.setPixelColor(0,0, 0xFF00FF);
       p.drawImage(0, 0, imag.scaled(width(),height()));
       std::this_thread::sleep_for(std::chrono::seconds(2));*/
    int h = height();
    int w = width();
    complex right_upper_corner = offset;
    complex shift(floor((double) right_upper_corner.real() / sub_image_size) * sub_image_size,
                  floor((double) right_upper_corner.imag() / sub_image_size) * sub_image_size);
    shift -= right_upper_corner;
    //        print(time_for_one_print, "time_for_one_print");
    //        print(print_time, "print_time");
    bool stop = true;
    std::vector<std::pair<complex, sub_image *>> not_ready;

    for (int y = 0; y <= h + sub_image_size; y += sub_image_size) {
        for (int x = 0; x <= w + sub_image_size; x += sub_image_size) {
            //            print("before before ");

            //            std::pair<int, int> k = {h + sub_image_size, w + sub_image_size};
            /*first = {int} 786
second = {int} 1105*/
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
                stop = false;
                not_ready.emplace_back(d, img);
                continue;
            }
            {
                std::lock_guard lock(img->m);
                p.drawImage(d.real(), d.imag(),
                            (img->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
            }
            //            print(c, "after");
        }
    }
    for (auto &pair : not_ready) {
        {
            std::lock_guard lock(pair.second->m);
            p.drawImage(pair.first.real(), pair.first.imag(),
                        (pair.second->getQImage()).scaled(sub_image_size, sub_image_size, Qt::KeepAspectRatio));
        }
    }
    //    print(count1, "counter");
    //    print(others, "others");

    if (!stop) {
        //        print("no need to stop");
        //        ++time_for_one_print;
        update();
    } else {
        //        print("need to stop");
        //        time_for_one_print = 1;
        //        ++print_time;
    }
}

template<class T>
void main_window::print(T &&t, std::string str) {
    std::cout << str << " " << t << "\n";
    std::cout.flush();
}

void main_window::resizeEvent(QResizeEvent *event) {
    update();
//    event->accept();
}

void main_window::wheelEvent(QWheelEvent *event) {
    /*  double mul = -1;
      if (event->angleDelta().y() < 0)
          mul = 1;
      double alpha = 0.4;
      alpha = 1 + mul * alpha;
      double scale = cache.worker.queue.get_scale();
      std::complex<double> x(event->position().x(), event->position().y());
      std::complex<double> offset_d(offset.real(), offset.imag());
      std::complex<double> betta = (x * scale * (1 - alpha) + offset_d * scale) / (offset_d * alpha * scale);
      offset *= betta;
      cache.change_scale(alpha);
      update();*/
    double mul = -1;
    if (event->angleDelta().y() < 0)
        mul = 1;
    double alpha1 = 0.1;
    alpha1 = 1 + mul * alpha1;
    double alpha = initial_scale;
    double betta = cache.worker.queue.get_scale();
    double gamma = betta * alpha1;
    std::complex<double> x(event->position().x(), event->position().y());
    std::complex<double> new_center = ((x + offset) * betta + center * alpha - gamma * x) / alpha;
    center = new_center;
    offset = complex(0, 0);
//    std::complex<double> x(event->position().x(), event->position().y());
//    std::complex<double> x(width()/2, height()/2);
//    std::complex<double> x1((x + offset) * betta);
//    std::complex<double> x2(center * alpha);
//    std::complex<double> new_center = ((x + offset) * betta + center * alpha) / alpha;
//    print(center, "prev");
//    center += new_center;
//    offset = complex(0, 0);
    cache.change_scale(alpha1);
    print(center, "new");
    update();

}




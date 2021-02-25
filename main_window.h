#pragma once

#include "image_cache.h"
#include <QMainWindow>
#include <QPoint>
#include <QWidget>
#include <atomic>
#include <complex>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
    class main_window;
}
QT_END_NAMESPACE

class main_window : public QMainWindow {
Q_OBJECT

private:
    using complex = std::complex<double>;

public:
    explicit main_window(QWidget *parent = nullptr);

    ~main_window() override;

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void changeEvent(QEvent *event) override;

private:
    std::unique_ptr<Ui::main_window> ui;
    const int detalization = 15;
    int sub_image_degree = 5;
    double initial_scale = 0.005;
    int prev_width;
    int prev_height;
    complex offset;
    image_cache cache;
    std::complex<double> center;

    bool is_pressed = false;

    QPoint drag_pos;

    void check_size_of_sub_images(bool forced = false);

    void accept_sub_image_resize(int new_degree);
};

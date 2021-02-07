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

    void wheelEvent(QWheelEvent *event) override;

private:
    std::unique_ptr<Ui::main_window> ui;
    int sub_image_size = 32;
    double initial_scale = 0.005;
    complex offset;
    image_cache cache;
    std::complex<double> center;
    bool is_pressed = false;
    QPoint drag_pos;
};


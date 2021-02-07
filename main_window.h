#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

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

public:
    using complex = std::complex<double>;
    main_window(QWidget* parent = nullptr);

    ~main_window() override;

    void mouseMoveEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent*) override;

    void mousePressEvent(QMouseEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

    void wheelEvent(QWheelEvent *event) override;

    //todo delete
    template <class T>
    static void print(T&& t, std::string str = "");

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

#endif // MAIN_WINDOW_H

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "image_cache.h"
#include <QMainWindow>
#include <QPoint>
#include <QSizePolicy>
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
    template <class T>
    static void print(T&& t, std::string str = "");

    int sub_image_size = 32;
//    double scale = 0.005;
//    uint64_t version = 0;

private:
    std::unique_ptr<Ui::main_window> ui;
    double initial_scale = 0.005;
    QPoint drag_pos;
    complex offset;
    std::complex<double> center;
    image_cache cache;
    bool is_pressed = false;
};

#endif // MAIN_WINDOW_H

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSizePolicy>
#include <QWidget>
#include <QPoint>
#include <memory>
#include <complex>
#include <atomic>
#include "image_cache.h"


QT_BEGIN_NAMESPACE
namespace Ui {
    class main_window;
}
QT_END_NAMESPACE

class main_window : public QMainWindow {
Q_OBJECT

public:

    using complex = std::complex<double>;
    main_window(QWidget *parent = nullptr);

    ~main_window() override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    QImage getImage(const complex & c);

    template<class T>
    static void print(T && t , std::string str = "");

    int sub_image_size = 32;
    double scale = 0.005;
private:
    size_t value(complex) const;

    std::unique_ptr<Ui::main_window> ui;
    QPoint drag_pos;
    std::complex<double> center_offset;
    image_cache cache;
};

#endif // MAIN_WINDOW_H

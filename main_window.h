#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSizePolicy>
#include <QWidget>
#include <QPoint>
#include <memory>
#include <complex>


QT_BEGIN_NAMESPACE
namespace Ui {
    class main_window;
}
QT_END_NAMESPACE

class main_window : public QMainWindow {
Q_OBJECT

public:
    main_window(QWidget *parent = nullptr);

    ~main_window() override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    QImage getImage(int x, int y);

    template<class T>
    static void print(T & t , std::string str = "");

private:

    size_t value(int, int, int, int) const;

    std::complex<double> center_offset;
    double scale = 0.005;
    QPoint drag_pos;

    std::unique_ptr<Ui::main_window> ui;

};

#endif // MAIN_WINDOW_H

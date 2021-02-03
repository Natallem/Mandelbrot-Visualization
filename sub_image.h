#pragma once

#include <complex>
#include <mutex>
#include <QImage>

class sub_image {

public:
    using complex = std::complex<double>;

    sub_image(const complex &vertex, double scale);

    int get_width() const;

    std::pair<complex, double> get_data() const ;

    void change_image(QImage &&new_image);

    QImage & getQImage();

private:
    mutable std::mutex m;
    const complex vertex;
    const double scale;
    QImage image;
    int size = 1;
};


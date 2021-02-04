#pragma once

#include <QImage>
#include <complex>
#include <mutex>

class sub_image {

public:
    using complex = std::complex<double>;

    sub_image(const complex& vertex, double scale);

    int get_width() const;

    std::pair<complex, double> get_data() const;

    void change_image(QImage&& new_image);

    QImage& getQImage();

    mutable std::mutex m;

private:
    const complex vertex;
    const double scale;
    QImage image;
    int size = 1;
};

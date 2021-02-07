#pragma once

#include <complex>
#include <mutex>
#include <QImage>

class sub_image {

public:
    using complex = std::complex<double>;

    explicit sub_image(const complex &vertex);

    int get_width() const;

    void change_image(QImage &&new_image);

    QImage &getImage();

    mutable std::mutex m;
    const complex vertex;
    QImage image;
};


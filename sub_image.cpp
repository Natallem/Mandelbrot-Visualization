#include "sub_image.h"

int sub_image::get_width() const {
    std::lock_guard lg(m);
    return image.width();
}

void sub_image::change_image(QImage &&new_image) {
    std::lock_guard lg(m);
    if (new_image.width() > image.width()) {
        image = std::move(new_image);
    }
}

sub_image::sub_image(const sub_image::complex &vertex) :
        vertex(vertex),
        image(QImage(1, 1, QImage::Format_RGB888)) {}

QImage &sub_image::getImage() {
    return image;
}



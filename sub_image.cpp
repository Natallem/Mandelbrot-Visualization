#include "sub_image.h"

int sub_image::get_width() const {
    std::lock_guard lg(m);
    return image.width();
}

std::pair<std::complex<double>, double> sub_image::get_data() const {
    //todo without loc as const
//    std::lock_guard lg(m);
//    return {vertex, scale};
}

void sub_image::change_image(QImage &&new_image) {
    std::lock_guard lg(m);
    if (new_image.width() > image.width()) {
        image = std::move(new_image);
    }
}

sub_image::sub_image(const sub_image::complex &vertex) :
        vertex(vertex),
        image(QImage(1, 1, QImage::Format_RGB888)) {
    std::lock_guard lg(m);
    if (image.width() == 1) {
        int black = 0;
        image.setPixelColor(0, 0, black);
    }
}

QImage &sub_image::getQImage()  {
    return image;
}



#include <iostream>
#include "sub_image.h"


void sub_image::cas_index(int cur_sub_image_degree) {
    while (true) {
        int cur_index = index_image;
        if (cur_index < cur_sub_image_degree) {
            if (index_image.compare_exchange_strong(cur_index, cur_sub_image_degree)) {
                return;
            }
        } else {
            return;
        }
    }
}

sub_image::sub_image(const sub_image::complex &vertex, int sub_image_degree, double scale) :
        vertex(vertex), sub_image_degree(sub_image_degree), scale(scale) {
    for (int i = 0; i <= sub_image_degree; i++) {
        int size = (1 << i);
        ready_images.emplace_back(size, size, QImage::Format_RGB888);
    }
    auto p = ready_images[0].bits();
    double step = double(1 << sub_image_degree) / 2;
    complex half(step, step);
    size_t color = value(vertex + half * scale);
    *p++ = 0;
    *p++ = color;
    *p = color;
}

QImage &sub_image::get_ready_image() {
    return ready_images[index_image];
}

sub_image::~sub_image() {
    while (working_threads != 0) {}
}

int sub_image::get_width() {
    return 1 << index_image;
}

void sub_image::create_new_image(std::atomic<uint64_t> &queue_version, uint64_t cur_version) {
    int sub_image_size = 1 << sub_image_degree;
    int cur_degree = index_image;
    int cur_size = 1 << cur_degree;
    if (cur_version == queue_version && cur_size != sub_image_size) {
        int step = sub_image_size / (cur_size * 2);
        update_image(ready_images[cur_degree + 1], step, cur_size * 2, queue_version, cur_version);
        if (cur_version == queue_version) {
            cas_index(cur_degree + 1);
        }
    }
    --working_threads;
}

void sub_image::update_image(QImage &img, int step, int size, std::atomic<uint64_t> &queue_version,
                             uint64_t cur_version) {
    int h = size;
    int w = size;
    size_t stride = img.bytesPerLine();
    unsigned char *data = img.bits();
    for (int y = 0; y != h; ++y) {
        unsigned char *p = data + y * stride;
        for (int x = 0; x != w; ++x) {
            if (cur_version != queue_version) {
                return;
            }
            size_t color = value(vertex + (complex(x * step, y * step) * scale));
            *p++ = 0;
            *p++ = color;
            *p++ = color;
        }
    }
}

size_t sub_image::value(complex point) {
    std::complex<double> z(0, 0);
    size_t const MAX_STEP = 255;
    for (size_t i = 1; i <= MAX_STEP; i++) {
        if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
            return i;
        }
        z = z * z + point;
    }
    return 0;
}


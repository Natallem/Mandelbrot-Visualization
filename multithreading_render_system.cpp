#include <iostream>
#include "multithreading_render_system.h"

void image_queue::add(sub_image &img) {
    std::lock_guard lg(m);
    int cur_width = img.get_width();
    pq.push({cur_width * 2, &img});
    not_empty.notify_all();
}

std::pair<sub_image *, uint64_t> image_queue::get_sub_image() {
    std::unique_lock lg(m);
    img_pair cur_pair = {-1, nullptr};
    not_empty.wait(lg, [&] {
        if (closed) {
            return true;
        }
        if (pq.empty()) {
            return false;
        }
        cur_pair = pq.top();
        pq.pop();
        return true;
    });
    if (closed) {
        return {nullptr, version};
    }
    int cur_size = cur_pair.second->get_width();
    if (cur_size != sub_image_size) {
        pq.push({cur_size * 2, cur_pair.second});
    }
    return {cur_pair.second, version};
}

image_queue::image_queue(int sub_image_size, double scale) : sub_image_size(sub_image_size), scale(scale) {}

void image_queue::close() {
    std::lock_guard lock(m);
    closed = true;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
    not_empty.notify_all();
}

void image_queue::change_scale(double d) {
    std::lock_guard lock(m);
    scale *= d;
    ++version;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
}

double image_queue::get_scale() const {
    std::lock_guard lock(m);
    return scale;
}

void multithreading_render_system::worker_proc() {
    while (true) {
        auto[cur_image, cur_version] = queue.get_sub_image();
        if (queue.closed) {
            break;
        }
        int cur_size = -1;
        {
            std::lock_guard lock(queue.m);
            if (cur_version == queue.version) {
                cur_size = cur_image->get_width();
            }
        }
        if (cur_size == -1 || cur_size == queue.sub_image_size) {
            continue;
        }
        complex vertex;
        double scale = 0;
        {
            std::lock_guard lock(queue.m);
            if (cur_version == queue.version) {
                vertex = cur_image->vertex;
                scale = queue.scale;
            }
        }
        if (!scale)
            continue;
        int step = queue.sub_image_size / (cur_size * 2);
        QImage new_image = std::move(getImage(vertex, scale, step, cur_size * 2));
        {
            std::lock_guard lock(queue.m);
            if (cur_version == queue.version) {
                cur_image->change_image(std::move(new_image));
            }
        }
    }
}

QImage multithreading_render_system::getImage(const complex &c, double scale, int step, int size) const {
//    assert(floor(abs(c.real())) == abs(c.real()));
//    assert(floor(abs(c.imag())) == abs(c.imag()));
//    assert(int(c.real()) % queue.sub_image_size == 0);
//    assert(int(c.imag()) % queue.sub_image_size == 0);
    int h = size;
    int w = size;
    QImage img(w, h, QImage::Format_RGB888);
    size_t stride = img.bytesPerLine();
    unsigned char *data = img.bits();
    for (int y = 0; y != h; ++y) {
        unsigned char *p = data + y * stride;
        for (int x = 0; x != w; ++x) {
            size_t color = value(c + (complex(x * step, y * step) * scale));
            *p++ = 0;
            *p++ = color;
            *p++ = color;
        }
    }
    img.width();
    return img;
}

size_t multithreading_render_system::value(complex point) {
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

multithreading_render_system::multithreading_render_system(size_t number_of_workers, int sub_image_size, double scale) :
        queue(sub_image_size, scale) {
    for (int i = 0; i < number_of_workers; i++)
        workers.emplace_back([this]() { worker_proc(); });
}

void multithreading_render_system::close() {
    queue.close();
    for (auto &thr : workers) {
        thr.join();
    }
}

multithreading_render_system::~multithreading_render_system() {
    close();
}

/*
uint64_t multithreading_render_system::get_vertion() {
    return version;
}
*/

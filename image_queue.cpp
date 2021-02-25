#include <iostream>
#include "render_system.h"

image_queue::image_queue(int sub_image_size, double scale) : sub_image_size(sub_image_size), scale(scale) {}

void image_queue::add(sub_image &img) {
    std::lock_guard lg(queue_mutex);
    pq.push({img.get_width() * 2, &img});
    not_empty.notify_all();
}

void image_queue::run_sub_image_creation() {
    std::unique_lock lg(queue_mutex);
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
        return;
    }
    int cur_size = cur_pair.second->get_width();
    if (cur_size != sub_image_size) {
        pq.push({cur_size * 2, cur_pair.second});
    } else {
        return;
    }
    ++cur_pair.second->working_threads;
    uint64_t cur_version = version;
    lg.unlock();
    cur_pair.second->create_new_image(version, cur_version);
}

double image_queue::get_scale() const {
    std::lock_guard lock(queue_mutex);
    return scale;
}

void image_queue::change_scale(double d) {
    std::lock_guard lock(queue_mutex);
    scale *= d;
    ++version;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
}

void image_queue::change_sub_image_size(int new_sub_image_size) {
    std::lock_guard lock(queue_mutex);
    ++version;
    sub_image_size = new_sub_image_size;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
}

void image_queue::close() {
    std::lock_guard lock(queue_mutex);
    closed = true;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
    not_empty.notify_all();
}

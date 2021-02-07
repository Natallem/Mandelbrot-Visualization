#include "render_system.h"

image_queue::image_queue(int sub_image_size, double scale) : sub_image_size(sub_image_size), scale(scale) {}

void image_queue::add(sub_image &img) {
    std::lock_guard lg(version_mutex);
    pq.push({img.get_width() * 2, &img});
    not_empty.notify_all();
}

std::pair<sub_image *, uint64_t> image_queue::get_sub_image() {
    std::unique_lock lg(version_mutex);
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

double image_queue::get_scale() const {
    std::lock_guard lock(version_mutex);
    return scale;
}

void image_queue::change_scale(double d) {
    std::lock_guard lock(version_mutex);
    scale *= d;
    ++version;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
}

void image_queue::close() {
    std::lock_guard lock(version_mutex);
    closed = true;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
    not_empty.notify_all();
}

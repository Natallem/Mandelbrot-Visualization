#include "image_queue.h"

image_queue::image_queue(int sub_image_degree, double scale) : sub_image_degree(sub_image_degree), scale(scale) {}

void image_queue::add(sub_image &img) {
    std::lock_guard lg(queue_mutex);
    pq.push({img.index_image +1, &img});
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
    int cur_degree = cur_pair.second->index_image;
    if (cur_degree != sub_image_degree){
        pq.push({cur_degree +1, cur_pair.second});
    }else {
        return;
    }
    ++cur_pair.second->working_threads;
    uint64_t cur_version = version;
    lg.unlock();
    cur_pair.second->render_sub_image(version, cur_version);
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

void image_queue::change_sub_image_degree(size_t new_sub_image_degree) {
    std::lock_guard lock(queue_mutex);
    ++version;
    sub_image_degree = new_sub_image_degree;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
}

void image_queue::close() {
    std::lock_guard lock(queue_mutex);
    closed = true;
    ++version;
    pq = std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>>();
    not_empty.notify_all();
}

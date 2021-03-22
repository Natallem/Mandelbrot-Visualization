#pragma once

#include "sub_image.h"
#include <condition_variable>
#include <queue>

class image_queue {

private:
    using img_pair = std::pair<int, sub_image *>;

public:
    image_queue(int sub_image_degree, double scale);

    ~image_queue() = default;

    void add(sub_image &img);

    void run_sub_image_creation();

    [[nodiscard]] double get_scale() const;

    void change_scale(double d);

    void close();

    void change_sub_image_degree(size_t new_sub_image_degree);

    std::atomic<bool> closed = false;

private:
    std::condition_variable not_empty;
    size_t sub_image_degree;
    mutable std::mutex queue_mutex;
    std::atomic<uint64_t> version = 0;
    double scale;
    std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>> pq;
};

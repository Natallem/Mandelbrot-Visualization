#pragma once

#include "sub_image.h"
#include <condition_variable>
#include <queue>
#include <thread>

class render_system;

class image_queue {
private:
    using img_pair = std::pair<int, sub_image*>;

public:
    image_queue(int sub_image_degree, double scale);

    void add(sub_image& img);

    void run_sub_image_creation();

    double get_scale() const;

    void change_scale(double d);

    void close();

    void change_sub_image_degree(size_t new_sub_image_degree);

private:
    std::condition_variable not_empty;
    std::atomic<bool> closed = false;
    size_t sub_image_degree;
    mutable std::mutex queue_mutex;
    std::atomic<uint64_t> version = 0;
    double scale;
    std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>> pq;

    friend render_system;
};

class render_system {
public:
    render_system(int number_of_workers, int sub_image_degree, double scale);

    ~render_system();

    image_queue& get_queue();

private:
    using complex = std::complex<double>;

    void worker_proc();

    image_queue queue;

    std::vector<std::thread> workers;
};

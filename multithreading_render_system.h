#pragma once

#include <queue>
#include <thread>
#include <condition_variable>
#include "sub_image.h"

class multithreading_render_system;

class image_queue {
private:
    using img_pair = std::pair<int, sub_image *>;
public:
    image_queue(int, double);

    image_queue() = default;

    void add(sub_image &img);

    std::pair<sub_image *, uint64_t> get_sub_image();

    void close();

    std::condition_variable not_empty;
    std::atomic<bool> closed = false;
    int sub_image_size;

    void change_scale(double d);
    double get_scale() const;

    mutable std::mutex m;
    uint64_t version = 0;
    double scale;
private:
    std::priority_queue<img_pair, std::vector<img_pair>, std::greater<>> pq;
};


class multithreading_render_system {
public:
    multithreading_render_system(size_t number_of_workers, int sub_image_size, double scale);

    ~multithreading_render_system();

    void close();


    image_queue queue;
private:

    using complex = std::complex<double>;

    void worker_proc();

    QImage getImage(const complex &c, double scale, int step, int size) const;

    static size_t value(complex point);

    std::vector<std::thread> workers;
};


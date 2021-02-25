#pragma once

#include <complex>
#include <mutex>
#include <QImage>

class sub_image {

public:
    using complex = std::complex<double>;

    explicit sub_image(const complex &vertex, int sub_image_degree, double scale);

    ~sub_image();

    QImage &get_ready_image();

    int get_width();

    size_t value(complex point);

    void create_new_image(std::atomic<uint64_t> &queue_version, uint64_t cur_version);

    std::atomic<size_t> working_threads = 0;
    std::atomic<int> index_image = 0;
private:
    const complex vertex;
    size_t sub_image_degree;

    double scale;
    std::vector<QImage> ready_images;

    void update_image(QImage &img, int step, int size, std::atomic<uint64_t> &queue_version, uint64_t cur_version);

    void cas_index(int cur_sub_image_degree);

};


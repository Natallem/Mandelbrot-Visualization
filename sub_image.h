#pragma once

#include <complex>
#include <mutex>
#include <QImage>

class sub_image {

public:
    using complex = std::complex<double>;

    explicit sub_image(const complex &vertex, size_t sub_image_degree, double scale);

    ~sub_image();

    QImage &get_ready_image();

    size_t get_size();

    void render_sub_image(std::atomic<uint64_t> &queue_version, uint64_t cur_version);

    std::atomic<size_t> working_threads = 0;

    std::atomic<size_t> index_image = 0;
private:
    static size_t color_value(complex point);

    void update_QImage(QImage &img, int step, int size, std::atomic<uint64_t> &queue_version, uint64_t cur_version);

    void cas_index(size_t cur_sub_image_degree);

    const complex vertex;
    const size_t sub_image_degree;
    const double scale;
    std::vector<QImage> ready_images;

};


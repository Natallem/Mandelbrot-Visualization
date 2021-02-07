#include "image_cache.h"

image_cache::image_cache(int sub_image_size, double scale, int thread_count) :
        worker(thread_count, sub_image_size, scale) {}

sub_image *image_cache::get_sub_image(complex d) {
    auto pair = cache.emplace(std::piecewise_construct,
                              std::forward_as_tuple(d.real(), d.imag()),
                              std::forward_as_tuple(d));
    if (pair.second) {
        worker.queue.add((*pair.first).second);
    }
    return &(*pair.first).second;
}

void image_cache::change_scale(double d) {
    worker.queue.change_scale(d);
    cache.clear();
}

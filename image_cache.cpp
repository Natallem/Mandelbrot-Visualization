#include "image_cache.h"

image_cache::image_cache(int sub_image_size, double scale, int thread_count) : scale(scale),
                                                                               worker(thread_count, sub_image_size) {

}

sub_image *image_cache::get_sub_image(int x, int y) {
    auto pair = cache.emplace(std::piecewise_construct,
                              std::forward_as_tuple(x, y),
                              std::forward_as_tuple(complex(x, y), scale));
    if (pair.second) {
        worker.queue.add((*pair.first).second);
    }
    return &(*pair.first).second;
}

void image_cache::clean() {

}

size_t image_cache::get_size() {
    return cache.size();
}




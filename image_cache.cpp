#include "image_cache.h"

image_cache::image_cache(int sub_image_degree, double scale, int thread_count) :
        renderer(thread_count, sub_image_degree, scale) {}

sub_image *image_cache::get_sub_image(complex d, size_t sub_image_degree) {
    auto pair = cache.emplace(std::piecewise_construct,
                              std::forward_as_tuple(d.real(), d.imag()),
                              std::forward_as_tuple(d, sub_image_degree, renderer.get_queue().get_scale()));
    if (pair.second) {
        renderer.get_queue().add((*pair.first).second);
    }
    return &(*pair.first).second;
}

void image_cache::change_scale(double d) {
    renderer.get_queue().change_scale(d);
    cache.clear();
}

double image_cache::get_cur_scale() {
    return renderer.get_queue().get_scale();
}

void image_cache::change_sub_image_degree(size_t i) {
    renderer.get_queue().change_sub_image_degree(i);
    cache.clear();
}

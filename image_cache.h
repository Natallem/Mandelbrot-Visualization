#pragma once

#include <unordered_map>
#include <map>
#include <complex>
#include "sub_image.h"
#include "render_system.h"

struct hash_pair {
    template<class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

class image_cache {
private:
    using complex = std::complex<double>;
public:

    image_cache(int sub_image_size, double scale, int thread_count);

    ~image_cache() = default;

    sub_image *get_sub_image(complex d);

    void change_scale(double d);

    double get_cur_scale();

    void change_sub_image_size(int i);

private:
    std::unordered_map<std::pair<double,double>, sub_image, hash_pair> cache;
    render_system renderer;
};

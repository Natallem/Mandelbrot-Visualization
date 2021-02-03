#pragma once

#include <unordered_map>
#include <map>
#include <complex>
#include "sub_image.h"
#include "multithreading_render_system.h"


//#include <bits/stdc++.h>

// A hash function used to hash a pair of any kind
struct hash_pair {
    template<class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

class image_cache {
public:
    using complex = std::complex<double>;

    ~image_cache() = default;


    image_cache(int sub_image_size, double scale, int thread_count);

    sub_image *get_sub_image(int x, int y);

    void clean();

    size_t get_size();

private:

    double scale;
    std::unordered_map<std::pair<int, int>, sub_image, hash_pair>  cache;
    multithreading_render_system worker;
};

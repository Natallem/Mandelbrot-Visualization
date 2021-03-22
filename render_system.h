#pragma once

#include "image_queue.h"
#include <thread>

class render_system {

public:
    render_system(int number_of_workers, int sub_image_degree, double scale);

    ~render_system();

    [[nodiscard]] image_queue& get_queue();

private:
    void worker_proc();

    image_queue queue;
    std::vector<std::thread> workers;
};

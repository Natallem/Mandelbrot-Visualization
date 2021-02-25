#include "render_system.h"

void render_system::worker_proc()
{
    while (true) {
        queue.run_sub_image_creation();
        if (queue.closed) {
            break;
        }
    }
}

render_system::render_system(int number_of_workers, int sub_image_degree, double scale)
    : queue(sub_image_degree, scale)
{
    for (int i = 0; i < number_of_workers; i++) { workers.emplace_back([this]() { worker_proc(); }); }
}

render_system::~render_system()
{
    queue.close();
    for (auto& thr : workers) {
        thr.join();
    }
}

image_queue& render_system::get_queue()
{
    return queue;
}

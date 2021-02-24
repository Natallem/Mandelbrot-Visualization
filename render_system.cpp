#include "render_system.h"

void render_system::worker_proc()
{
    while (true) {
        auto [cur_image, cur_version] = queue.get_sub_image();
        if (queue.closed) {
            break;
        }
        int cur_size = -1;
        {
            std::lock_guard lock(queue.queue_mutex);
            if (cur_version == queue.version) {
                cur_size = cur_image->get_width();
            }
        }
        if (cur_size == -1 || cur_size == queue.sub_image_size) {
            continue;
        }
        complex vertex;
        double scale = 0;
        {
            std::lock_guard lock(queue.queue_mutex);
            if (cur_version == queue.version) {
                vertex = cur_image->vertex;
                scale = queue.scale;
            }
        }
        if (!scale)
            continue;
        int step = queue.sub_image_size / (cur_size * 2);
        QImage new_image = std::move(getImage(vertex, scale, step, cur_size * 2, cur_version));
        {
            std::lock_guard lock(queue.queue_mutex);
            if (cur_version == queue.version) {
                cur_image->change_image(std::move(new_image));
            }
        }
    }
}

QImage render_system::getImage(const complex& c, double scale, int step, int size, uint64_t cur_version)
{
    int h = size;
    int w = size;
    QImage img(w, h, QImage::Format_RGB888);
    size_t stride = img.bytesPerLine();
    unsigned char* data = img.bits();
    for (int y = 0; y != h; ++y) {

        unsigned char* p = data + y * stride;
        for (int x = 0; x != w; ++x) {
            if (cur_version != queue.version) {
                return img;
            }
            size_t color = value(c + (complex(x * step, y * step) * scale));
            *p++ = 0;
            *p++ = color;
            *p++ = color;
        }
    }
    return img;
}

size_t render_system::value(complex point)
{
    std::complex<double> z(0, 0);
    size_t const MAX_STEP = 255;
    for (size_t i = 1; i <= MAX_STEP; i++) {
        if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
            return i;
        }
        z = z * z + point;
    }
    return 0;
}

render_system::render_system(int number_of_workers, int sub_image_size, double scale)
    : queue(sub_image_size, scale)
{
    for (int i = 0; i < number_of_workers; i++)
        workers.emplace_back([this]() { worker_proc(); });
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

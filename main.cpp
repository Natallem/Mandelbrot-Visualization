#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    bool debug = true;
    if (debug) {
        int i = 1;
        while (i) {
            i++;
            QApplication app(argc, argv);
            main_window w;
            w.show();
            std::thread th([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                try {
                    app.quit();
                } catch (...) {
                    int x = 10;
                    throw;
                }
            });
            try {
                app.exec();

            } catch (...) {
                int x = 10;
                throw;
            }
            th.join();
            if (i == 100)
                break;
        }
        return 0;
    } else {
        QApplication app(argc, argv);
        main_window w;
        w.show();
        return app.exec();
    }
}

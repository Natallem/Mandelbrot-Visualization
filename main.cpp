#include "main_window.h"

#include <QApplication>
#include <iostream>

int main(int argc, char* argv[])
{
    bool debug = false;
    if (debug) {
        int i = 1;
        while (i) {
            i++;
            std::cout <<"\nrun number " << i << "\n";
            std::cout.flush();
            QApplication app(argc, argv);
            main_window w;
            w.show();
            std::thread th([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                app.quit();
            });
            app.exec();
            th.join();
        }
        return 0;
    } else {
        QApplication app(argc, argv);
        main_window w;
        w.show();
        //        std::thread th([&]() {
        //            std::this_thread::sleep_for(std::chrono::seconds(1));
        //            app.quit();
        //        });
        //
        //        th.join();
        return app.exec();
    }
}

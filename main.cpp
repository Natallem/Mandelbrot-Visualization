#include "main_window.h"

#include <QApplication>
#include <Windows.h>
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    main_window w;
    w.show();
    return app.exec();
}

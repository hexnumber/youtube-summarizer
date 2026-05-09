#include <QApplication>
#include <curl/curl.h>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    QApplication app(argc, argv);
    app.setStyle("Fusion");

    MainWindow window;
    window.resize(900, 700);
    window.show();

    int result = app.exec();

    curl_global_cleanup();
    return result;
}

#include <QApplication>
#include "simulator.h"
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_ALL);
    QApplication app(argc, argv);
    DDoSSimulator window;
    window.show();
    const int result = QApplication::exec();
    curl_global_cleanup();
    return result;
}

#include <QApplication>
#include "simulator.h"
#include "webscanner_tab.h"
#include "style.h"
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_ALL);
    QApplication app(argc, argv);
    app.setStyleSheet(Style::darkTheme());

    const auto tab_widget = new QTabWidget;
    tab_widget->addTab(new DDoSSimulator, "DDoS Simulator");
    tab_widget->addTab(new WebScannerTab, "Web Scanner");
    tab_widget->setWindowTitle("Network Tool");
    tab_widget->resize(1280, 720);
    tab_widget->show();

    const int result = QApplication::exec();
    curl_global_cleanup();

    delete tab_widget;
    return result;
}

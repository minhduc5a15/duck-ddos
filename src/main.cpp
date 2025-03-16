#include <QApplication>
#include "simulator.h"
#include "webscanner_tab.h"
#include "style.h"
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_ALL);
    QApplication app(argc, argv);
    app.setStyleSheet(Style::darkTheme());

    const auto tabWidget = new QTabWidget;
    tabWidget->addTab(new DDoSSimulator, "DDoS Simulator");
    tabWidget->addTab(new WebScannerTab, "Web Scanner");
    tabWidget->setWindowTitle("Network Tool");
    tabWidget->resize(800, 640);
    tabWidget->show();

    const int result = QApplication::exec();
    curl_global_cleanup();

    delete tabWidget;
    return result;
}
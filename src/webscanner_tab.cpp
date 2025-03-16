#include "webscanner_tab.h"
#include "webscanner.h"
#include "proxy.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

WebScannerTab::WebScannerTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    resultTimer = new QTimer(this);
    connect(resultTimer, &QTimer::timeout, this, &WebScannerTab::updateScanResult);
    resultTimer->start(1000);
}

void WebScannerTab::setupUi() {
    urlInput = new QLineEdit(this);
    urlInput->setPlaceholderText("Enter target URL (e.g., http://example.com)");

    scanType = new QComboBox(this);
    scanType->addItem("Headers");

    threadSlider = new QSlider(Qt::Horizontal, this);
    threadSlider->setRange(1, 10);
    threadSlider->setValue(1);

    proxyInput = new QLineEdit(this);
    proxyInput->setPlaceholderText("Proxy IP:Port or file path");

    useProxyCheck = new QCheckBox("Use Proxy", this);

    headerInput = new QLineEdit(this);
    headerInput->setPlaceholderText("Custom Header (e.g., Cookie: id=123)");

    sslVerifyCheck = new QCheckBox("Verify SSL", this);
    sslVerifyCheck->setChecked(true);

    startButton = new QPushButton("Start Scan", this);
    stopButton = new QPushButton("Stop Scan", this);

    resultDisplay = new QTextEdit(this);
    resultDisplay->setReadOnly(true);

    auto *layout = new QGridLayout;
    layout->addWidget(new QLabel("Target URL:"), 0, 0);
    layout->addWidget(urlInput, 0, 1, 1, 2);
    layout->addWidget(new QLabel("Scan Type:"), 1, 0);
    layout->addWidget(scanType, 1, 1, 1, 2);
    layout->addWidget(new QLabel("Threads:"), 2, 0);
    layout->addWidget(threadSlider, 2, 1, 1, 2);
    layout->addWidget(new QLabel("Proxy (optional):"), 3, 0);
    layout->addWidget(proxyInput, 3, 1);
    layout->addWidget(useProxyCheck, 3, 2);
    layout->addWidget(new QLabel("Custom Header:"), 4, 0);
    layout->addWidget(headerInput, 4, 1, 1, 2);
    layout->addWidget(startButton, 5, 0);
    layout->addWidget(stopButton, 5, 1);
    layout->addWidget(sslVerifyCheck, 5, 2);
    layout->addWidget(resultDisplay, 6, 0, 1, 3);
    setLayout(layout);

    connect(startButton, &QPushButton::clicked, this, &WebScannerTab::startScan);
    connect(stopButton, &QPushButton::clicked, this, &WebScannerTab::stopScan);

}

void WebScannerTab::log(const QString &message) const {
    resultDisplay->append(message);
}

bool WebScannerTab::validateUrl(const QString &url) const {
    static const QRegularExpression re(
        "^(http|https)://([a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,}|localhost|\\d+\\.\\d+\\.\\d+\\.\\d+)(:\\d+)?(/.*)?$"
    );
    return re.match(url).hasMatch();
}

void WebScannerTab::startScan() {
    if (urlInput->text().isEmpty() || !validateUrl(urlInput->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL!");
        return;
    }

    const std::string url = urlInput->text().toStdString();
    const std::string type = scanType->currentText().toStdString();
    const int threads = threadSlider->value();
    const std::string customHeader = headerInput->text().toStdString();
    const bool verifySSL = sslVerifyCheck->isChecked();
    ProxyManager proxy;

    if (useProxyCheck->isChecked()) {
        const QString proxyText = proxyInput->text();
        proxy.setProxy(proxyText.toStdString());
    }
    proxy.enable(useProxyCheck->isChecked());

    log(QString("Starting %1 scan on %2").arg(type.c_str(), url.c_str()));
    WebScanner::start(url, type, threads, customHeader, verifySSL, proxy);
}

void WebScannerTab::stopScan() const {
    WebScanner::stop();
    log("Scan stopped.");
}

void WebScannerTab::updateScanResult() const {
    const std::string result = WebScanner::getScanResult();
    if (!result.empty()) {
        log(QString::fromStdString(result));
    }
}

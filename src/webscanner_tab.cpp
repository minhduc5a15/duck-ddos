#include "webscanner_tab.h"
#include "webscanner.h"
#include "proxy.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

WebScannerTab::WebScannerTab(QWidget *parent) : QWidget(parent) {
    setup_ui();
    result_timer_ = new QTimer(this);
    connect(result_timer_, &QTimer::timeout, this, &WebScannerTab::update_scan_result);
    result_timer_->start(1000);
}

void WebScannerTab::setup_ui() {
    url_input_ = new QLineEdit(this);
    url_input_->setPlaceholderText("Enter target URL (e.g., http://example.com)");

    scan_type_ = new QComboBox(this);
    scan_type_->addItem("Headers");

    thread_slider_ = new QSlider(Qt::Horizontal, this);
    thread_slider_->setRange(1, 10);
    thread_slider_->setValue(1);

    proxy_input_ = new QLineEdit(this);
    proxy_input_->setPlaceholderText("Proxy IP:Port or file path");

    use_proxy_check_ = new QCheckBox("Use Proxy", this);

    header_input_ = new QLineEdit(this);
    header_input_->setPlaceholderText("Custom Header (e.g., Cookie: id=123)");

    ssl_verify_check_ = new QCheckBox("Verify SSL", this);
    ssl_verify_check_->setChecked(true);

    start_button_ = new QPushButton("Start Scan", this);
    stop_button_ = new QPushButton("Stop Scan", this);

    result_display_ = new QTextEdit(this);
    result_display_->setReadOnly(true);

    auto *layout = new QGridLayout;
    layout->addWidget(new QLabel("Target URL:"), 0, 0);
    layout->addWidget(url_input_, 0, 1, 1, 2);
    layout->addWidget(new QLabel("Scan Type:"), 1, 0);
    layout->addWidget(scan_type_, 1, 1, 1, 2);
    layout->addWidget(new QLabel("Threads:"), 2, 0);
    layout->addWidget(thread_slider_, 2, 1, 1, 2);
    layout->addWidget(new QLabel("Proxy (optional):"), 3, 0);
    layout->addWidget(proxy_input_, 3, 1);
    layout->addWidget(use_proxy_check_, 3, 2);
    layout->addWidget(new QLabel("Custom Header:"), 4, 0);
    layout->addWidget(header_input_, 4, 1, 1, 2);
    layout->addWidget(start_button_, 5, 0);
    layout->addWidget(stop_button_, 5, 1);
    layout->addWidget(ssl_verify_check_, 5, 2);
    layout->addWidget(result_display_, 6, 0, 1, 3);
    setLayout(layout);

    connect(start_button_, &QPushButton::clicked, this, &WebScannerTab::start_scan);
    connect(stop_button_, &QPushButton::clicked, this, &WebScannerTab::stop_scan);
}

void WebScannerTab::log(const QString &message) const {
    result_display_->append(message);
}

bool WebScannerTab::validate_url(const QString &url) const {
    static const QRegularExpression re(
        "^(http|https)://([a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,}|localhost|\\d+\\.\\d+\\.\\d+\\.\\d+)(:\\d+)?(/.*)?$"
    );
    return re.match(url).hasMatch();
}

void WebScannerTab::start_scan() {
    if (url_input_->text().isEmpty() || !validate_url(url_input_->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL!");
        return;
    }

    const std::string url = url_input_->text().toStdString();
    const std::string type = scan_type_->currentText().toStdString();
    const int threads = thread_slider_->value();
    const std::string custom_header = header_input_->text().toStdString();
    const bool verifySSL = ssl_verify_check_->isChecked();
    ProxyManager proxy;

    if (use_proxy_check_->isChecked()) {
        const QString proxy_text = proxy_input_->text();
        proxy.set_proxy(proxy_text.toStdString());
    }
    proxy.enable(use_proxy_check_->isChecked());

    log(QString("Starting %1 scan on %2").arg(type.c_str(), url.c_str()));
    WebScanner::start(url, type, threads, custom_header, verifySSL, proxy);
}

void WebScannerTab::stop_scan() const {
    WebScanner::stop();
    log("Scan stopped.");
}

void WebScannerTab::update_scan_result() const {
    const std::string result = WebScanner::get_scan_result();
    if (!result.empty()) {
        log(QString::fromStdString(result));
    }
}

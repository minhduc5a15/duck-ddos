#include "simulator.h"
#include "attacker.h"
#include "proxy.h"
#include "style.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

DDoSSimulator::DDoSSimulator(QWidget *parent) : QWidget(parent) {
    setup_ui();
    status_timer_ = new QTimer(this);
    connect(status_timer_, &QTimer::timeout, this, &DDoSSimulator::update_status);
    status_timer_->start(1000);
}

void DDoSSimulator::setup_ui() {
    setWindowTitle("DDoS Simulator");

    url_input_ = new QLineEdit(this);
    url_input_->setPlaceholderText("Enter target URL (e.g., http://localhost)");

    attack_type_ = new QComboBox(this);
    attack_type_->addItem("HTTP Flood");
    attack_type_->addItem("Slowloris");

    thread_slider_ = new QSlider(Qt::Horizontal, this);
    thread_slider_->setRange(1, 100);
    thread_slider_->setValue(5);

    request_count_ = new QSpinBox(this);
    request_count_->setRange(10, 10000);
    request_count_->setValue(100);

    rps_input_ = new QSpinBox(this);
    rps_input_->setRange(1, 10000);
    rps_input_->setValue(100);

    proxy_input_ = new QLineEdit(this);
    proxy_input_->setPlaceholderText("Proxy IP:Port or file path (e.g., proxies.txt)");

    use_proxy_check_ = new QCheckBox("Use Proxy", this);

    header_input_ = new QLineEdit(this);
    header_input_->setPlaceholderText("Custom Header (e.g., Cookie: id=123)");

    ssl_verify_check_ = new QCheckBox("Verify SSL", this);
    ssl_verify_check_->setChecked(true);

    start_button_ = new QPushButton("Start", this);
    stop_button_ = new QPushButton("Stop", this);
    pause_button_ = new QPushButton("Pause", this);
    clear_button_ = new QPushButton("Clear Log", this);
    load_proxy_button_ = new QPushButton("Load Proxy File", this);

    log_display_ = new QTextEdit(this);
    log_display_->setReadOnly(true);

    auto *layout = new QGridLayout;
    layout->addWidget(new QLabel("Target URL:"), 0, 0);
    layout->addWidget(url_input_, 0, 1, 1, 3);
    layout->addWidget(new QLabel("Attack Type:"), 1, 0);
    layout->addWidget(attack_type_, 1, 1, 1, 3);
    layout->addWidget(new QLabel("Threads:"), 2, 0);
    layout->addWidget(thread_slider_, 2, 1, 1, 3);
    layout->addWidget(new QLabel("Requests:"), 3, 0);
    layout->addWidget(request_count_, 3, 1);
    layout->addWidget(new QLabel("RPS:"), 3, 2);
    layout->addWidget(rps_input_, 3, 3);
    layout->addWidget(new QLabel("Proxy (optional):"), 4, 0);
    layout->addWidget(proxy_input_, 4, 1, 1, 2);
    layout->addWidget(use_proxy_check_, 4, 3);
    layout->addWidget(new QLabel("Custom Header:"), 5, 0);
    layout->addWidget(header_input_, 5, 1, 1, 3);
    layout->addWidget(start_button_, 6, 0);
    layout->addWidget(stop_button_, 6, 1);
    layout->addWidget(pause_button_, 6, 2);
    layout->addWidget(ssl_verify_check_, 6, 3);
    layout->addWidget(clear_button_, 7, 0);
    layout->addWidget(load_proxy_button_, 7, 1, 1, 3);
    layout->addWidget(log_display_, 8, 0, 1, 4);
    setLayout(layout);

    connect(start_button_, &QPushButton::clicked, this, &DDoSSimulator::start_attack);
    connect(stop_button_, &QPushButton::clicked, this, &DDoSSimulator::stop_attack);
    connect(pause_button_, &QPushButton::clicked, this, &DDoSSimulator::pause_attack);
    connect(clear_button_, &QPushButton::clicked, this, &DDoSSimulator::clear_log);
    connect(load_proxy_button_, &QPushButton::clicked, this, &DDoSSimulator::load_proxy_file);
}

void DDoSSimulator::log(const QString &message) const {
    const QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    log_display_->append(timestamp + message);
}

bool DDoSSimulator::validate_url(const QString &url) const {
    static const QRegularExpression re(
        "^(http|https)://("
        "([a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,})|"
        "(localhost)|"
        "(\\d+\\.\\d+\\.\\d+\\.\\d+)"
        ")(:\\d+)?(/.*)?$"
    );
    return re.match(url).hasMatch();
}

bool DDoSSimulator::validate_proxy(const QString &proxy) const {
    static const QRegularExpression re(R"(^\d+\.\d+\.\d+\.\d+:\d+$)");
    return proxy.isEmpty() || re.match(proxy).hasMatch();
}

void DDoSSimulator::start_attack() {
    if (url_input_->text().isEmpty() || !validate_url(url_input_->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL (e.g., http://example.com)!");
        return;
    }
    if (use_proxy_check_->isChecked() && !validate_proxy(proxy_input_->text()) && !QFile::exists(proxy_input_->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid proxy (e.g., 103.221.222.222:8080) or proxy file!");
        return;
    }

    const std::string url = url_input_->text().toStdString();
    const std::string type = attack_type_->currentText().toStdString();
    const int threads = thread_slider_->value();
    const int requests = request_count_->value();
    const int rps = rps_input_->value();
    const std::string customHeader = header_input_->text().toStdString();
    const bool verifySSL = ssl_verify_check_->isChecked();
    ProxyManager proxy;

    if (use_proxy_check_->isChecked()) {
        const QString proxy_text = proxy_input_->text();
        if (QFile::exists(proxy_text)) {
            QFile file(proxy_text);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                std::vector<std::string> proxies, live_proxies;
                while (!in.atEnd()) proxies.push_back(in.readLine().trimmed().toStdString());
                file.close();
                for (const auto &p: proxies) {
                    if (proxy.test_proxy(p)) live_proxies.push_back(p);
                }
                proxy.set_proxy_list(live_proxies);
                log(QString("Loaded %1 live proxies from %2").arg(live_proxies.size()).arg(proxies.size()));
            }
        }
        else {
            proxy.set_proxy(proxy_text.toStdString());
        }
    }
    proxy.enable(use_proxy_check_->isChecked());

    log(QString("Starting %1 attack on %2 with %3 threads")
        .arg(type.c_str(), url.c_str()).arg(threads));
    if (type == "HTTP Flood") {
        log(QString("Sending %1 requests per thread at %2 RPS").arg(requests).arg(rps));
    }
    if (proxy.is_enabled()) {
        log(QString("Using %1 proxies").arg(proxy.proxy_count()));
    }
    if (!customHeader.empty()) {
        log(QString("Using custom header: %1").arg(customHeader.c_str()));
    }

    Attacker::start(url, type, threads, requests, rps, customHeader, verifySSL, proxy);
    pause_button_->setText("Pause");
}

void DDoSSimulator::stop_attack() const {
    Attacker::stop();
    log("Attack stopped.");
}

void DDoSSimulator::pause_attack() {
    if (!Attacker::is_attacking()) {
        QMessageBox::information(this, "Info", "No attack is running!");
        return;
    }
    Attacker::pause();
    if (Attacker::is_paused()) {
        log("Attack paused.");
        pause_button_->setText("Resume");
    }
    else {
        log("Attack resumed.");
        pause_button_->setText("Pause");
    }
}

void DDoSSimulator::clear_log() const {
    log_display_->clear();
}

void DDoSSimulator::load_proxy_file() {
    const QString fileName = QFileDialog::getOpenFileName(this, "Load Proxy List", "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        proxy_input_->setText(fileName);
        log(QString("Selected proxy file: %1").arg(fileName));
    }
}

void DDoSSimulator::update_status() const {
    if (Attacker::is_attacking()) {
        log(QString("Status: Successful requests: %1 | Failed requests: %2")
            .arg(Attacker::get_successful_requests())
            .arg(Attacker::get_failed_requests()));
    }
}

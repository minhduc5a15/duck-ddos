#include "simulator.h"
#include "attacker.h"
#include "proxy.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QRegularExpression>

DDoSSimulator::DDoSSimulator(QWidget* parent) : QWidget(parent) {
    setupUi();
    applyStyle();
}

void DDoSSimulator::setupUi() {
    setWindowTitle("DDoS Simulator");
    setMinimumSize(600, 450);

    urlInput = new QLineEdit(this);
    urlInput->setPlaceholderText("Enter target URL (e.g., http://localhost)");

    attackType = new QComboBox(this);
    attackType->addItem("HTTP Flood");
    attackType->addItem("Slowloris");

    threadSlider = new QSlider(Qt::Horizontal, this);
    threadSlider->setRange(1, 50);
    threadSlider->setValue(5);

    requestCount = new QSpinBox(this);
    requestCount->setRange(10, 1000);
    requestCount->setValue(100);

    proxyInput = new QLineEdit(this);
    proxyInput->setPlaceholderText("Proxy IP:Port (e.g., 103.221.222.222:8080)");

    useProxyCheck = new QCheckBox("Use Proxy", this);

    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    pauseButton = new QPushButton("Pause", this);
    clearButton = new QPushButton("Clear Log", this);

    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);

    auto* layout = new QGridLayout;
    layout->addWidget(new QLabel("Target URL:"), 0, 0);
    layout->addWidget(urlInput, 0, 1, 1, 3);
    layout->addWidget(new QLabel("Attack Type:"), 1, 0);
    layout->addWidget(attackType, 1, 1, 1, 3);
    layout->addWidget(new QLabel("Threads:"), 2, 0);
    layout->addWidget(threadSlider, 2, 1, 1, 3);
    layout->addWidget(new QLabel("Requests (HTTP Flood):"), 3, 0);
    layout->addWidget(requestCount, 3, 1, 1, 3);
    layout->addWidget(new QLabel("Proxy (optional):"), 4, 0);
    layout->addWidget(proxyInput, 4, 1, 1, 2);
    layout->addWidget(useProxyCheck, 4, 3);
    layout->addWidget(startButton, 5, 0);
    layout->addWidget(stopButton, 5, 1);
    layout->addWidget(pauseButton, 5, 2);
    layout->addWidget(clearButton, 5, 3);
    layout->addWidget(logDisplay, 6, 0, 1, 4);
    setLayout(layout);

    connect(startButton, &QPushButton::clicked, this, &DDoSSimulator::startAttack);
    connect(stopButton, &QPushButton::clicked, this, &DDoSSimulator::stopAttack);
    connect(pauseButton, &QPushButton::clicked, this, &DDoSSimulator::pauseAttack);
    connect(clearButton, &QPushButton::clicked, this, &DDoSSimulator::clearLog);
}

void DDoSSimulator::applyStyle() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #d4d4d4;
            font-family: "Arial", sans-serif;
        }
        QLineEdit, QComboBox, QSpinBox {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 5px;
            padding: 5px;
            color: #d4d4d4;
        }
        QPushButton {
            background-color: #5c85d6;
            color: #ffffff;
            border: none;
            padding: 8px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #476bb3;
        }
        QTextEdit {
            background-color: #1e1e1e;
            border: 1px solid #555555;
            border-radius: 5px;
            color: #d4d4d4;
        }
        QSlider::groove:horizontal {
            background: #555555;
            height: 8px;
        }
        QSlider::handle:horizontal {
            background: #5c85d6;
            width: 16px;
        }
        QCheckBox {
            color: #d4d4d4;
        }
    )");
}

void DDoSSimulator::log(const QString& message) const {
    const QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    logDisplay->append(timestamp + message);
}

bool DDoSSimulator::validateUrl(const QString& url) const {
    static const QRegularExpression re(
        "^(http|https)://("
        "([a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,})|" // Domain công khai
        "(localhost)|"                        // Hỗ trợ localhost
        "(\\d+\\.\\d+\\.\\d+\\.\\d+)"        // Hỗ trợ địa chỉ IP
        ")(:\\d+)?(/.*)?$"                    // Cổng và path tùy chọn
    );
    return re.match(url).hasMatch();
}

bool DDoSSimulator::validateProxy(const QString& proxy) const {
    static const QRegularExpression re(R"(^\d+\.\d+\.\d+\.\d+:\d+$)");
    return proxy.isEmpty() || re.match(proxy).hasMatch();
}

void DDoSSimulator::startAttack() {
    if (urlInput->text().isEmpty() || !validateUrl(urlInput->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL (e.g., http://example.com)!");
        return;
    }
    if (useProxyCheck->isChecked() && !validateProxy(proxyInput->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid proxy (e.g., 103.221.222.222:8080)!");
        return;
    }

    const std::string url = urlInput->text().toStdString();
    const std::string type = attackType->currentText().toStdString();
    const int threads = threadSlider->value();
    const int requests = requestCount->value();
    ProxyManager proxy(proxyInput->text().toStdString());
    proxy.enable(useProxyCheck->isChecked());

    log(QString("Starting %1 attack on %2 with %3 threads")
        .arg(type.c_str(), url.c_str()).arg(threads));
    if (type == "HTTP Flood") {
        log(QString("Sending %1 requests per thread").arg(requests));
    }
    if (proxy.isEnabled() && !proxy.getProxy().empty()) {
        log(QString("Using proxy: %1").arg(proxy.getProxy().c_str()));
    }

    Attacker::start(url, type, threads, requests, proxy);
    pauseButton->setText("Pause");
}

void DDoSSimulator::stopAttack() const {
    Attacker::stop();
    log("Attack stopped.");
}

void DDoSSimulator::pauseAttack() {
    if (!Attacker::isAttacking()) {
        QMessageBox::information(this, "Info", "No attack is running!");
        return;
    }
    Attacker::pause();
    if (Attacker::isPaused()) {
        log("Attack paused.");
        pauseButton->setText("Resume");
    } else {
        log("Attack resumed.");
        pauseButton->setText("Pause");
    }
}

void DDoSSimulator::clearLog() const {
    logDisplay->clear();
}
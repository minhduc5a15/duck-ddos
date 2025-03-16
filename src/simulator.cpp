#include "simulator.h"
#include "attacker.h"
#include "proxy.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

DDoSSimulator::DDoSSimulator(QWidget *parent) : QWidget(parent) {
    setupUi();
    applyStyle();
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &DDoSSimulator::updateStatus);
    statusTimer->start(1000);
}

void DDoSSimulator::setupUi() {
    setWindowTitle("DDoS Simulator");
    setMinimumSize(600, 500);

    urlInput = new QLineEdit(this);
    urlInput->setPlaceholderText("Enter target URL (e.g., http://localhost)");

    attackType = new QComboBox(this);
    attackType->addItem("HTTP Flood");
    attackType->addItem("Slowloris");

    threadSlider = new QSlider(Qt::Horizontal, this);
    threadSlider->setRange(1, 100);
    threadSlider->setValue(5);

    requestCount = new QSpinBox(this);
    requestCount->setRange(10, 10000);
    requestCount->setValue(100);

    rpsInput = new QSpinBox(this);
    rpsInput->setRange(1, 10000);
    rpsInput->setValue(100);

    proxyInput = new QLineEdit(this);
    proxyInput->setPlaceholderText("Proxy IP:Port or file path (e.g., proxies.txt)");

    useProxyCheck = new QCheckBox("Use Proxy", this);

    headerInput = new QLineEdit(this);
    headerInput->setPlaceholderText("Custom Header (e.g., Cookie: id=123)");

    sslVerifyCheck = new QCheckBox("Verify SSL", this);
    sslVerifyCheck->setChecked(true);

    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    pauseButton = new QPushButton("Pause", this);
    clearButton = new QPushButton("Clear Log", this);
    loadProxyButton = new QPushButton("Load Proxy File", this);

    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);

    auto *layout = new QGridLayout;
    layout->addWidget(new QLabel("Target URL:"), 0, 0);
    layout->addWidget(urlInput, 0, 1, 1, 3);
    layout->addWidget(new QLabel("Attack Type:"), 1, 0);
    layout->addWidget(attackType, 1, 1, 1, 3);
    layout->addWidget(new QLabel("Threads:"), 2, 0);
    layout->addWidget(threadSlider, 2, 1, 1, 3);
    layout->addWidget(new QLabel("Requests:"), 3, 0);
    layout->addWidget(requestCount, 3, 1);
    layout->addWidget(new QLabel("RPS:"), 3, 2);
    layout->addWidget(rpsInput, 3, 3);
    layout->addWidget(new QLabel("Proxy (optional):"), 4, 0);
    layout->addWidget(proxyInput, 4, 1, 1, 2);
    layout->addWidget(useProxyCheck, 4, 3);
    layout->addWidget(new QLabel("Custom Header:"), 5, 0);
    layout->addWidget(headerInput, 5, 1, 1, 3);
    layout->addWidget(startButton, 6, 0);
    layout->addWidget(stopButton, 6, 1);
    layout->addWidget(pauseButton, 6, 2);
    layout->addWidget(sslVerifyCheck, 6, 3);
    layout->addWidget(clearButton, 7, 0);
    layout->addWidget(loadProxyButton, 7, 1, 1, 3);
    layout->addWidget(logDisplay, 8, 0, 1, 4);
    setLayout(layout);

    connect(startButton, &QPushButton::clicked, this, &DDoSSimulator::startAttack);
    connect(stopButton, &QPushButton::clicked, this, &DDoSSimulator::stopAttack);
    connect(pauseButton, &QPushButton::clicked, this, &DDoSSimulator::pauseAttack);
    connect(clearButton, &QPushButton::clicked, this, &DDoSSimulator::clearLog);
    connect(loadProxyButton, &QPushButton::clicked, this, &DDoSSimulator::loadProxyFile);
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
            border-radius: 3px;
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

void DDoSSimulator::log(const QString &message) const {
    const QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    logDisplay->append(timestamp + message);
}

bool DDoSSimulator::validateUrl(const QString &url) const {
    static const QRegularExpression re(
        "^(http|https)://("
        "([a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,})|"
        "(localhost)|"
        "(\\d+\\.\\d+\\.\\d+\\.\\d+)"
        ")(:\\d+)?(/.*)?$"
    );
    return re.match(url).hasMatch();
}

bool DDoSSimulator::validateProxy(const QString &proxy) const {
    static const QRegularExpression re(R"(^\d+\.\d+\.\d+\.\d+:\d+$)");
    return proxy.isEmpty() || re.match(proxy).hasMatch();
}

void DDoSSimulator::startAttack() {
    if (urlInput->text().isEmpty() || !validateUrl(urlInput->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL (e.g., http://example.com)!");
        return;
    }
    if (useProxyCheck->isChecked() && !validateProxy(proxyInput->text()) && !QFile::exists(proxyInput->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid proxy (e.g., 103.221.222.222:8080) or proxy file!");
        return;
    }

    const std::string url = urlInput->text().toStdString();
    const std::string type = attackType->currentText().toStdString();
    const int threads = threadSlider->value();
    const int requests = requestCount->value();
    const int rps = rpsInput->value();
    const std::string customHeader = headerInput->text().toStdString();
    const bool verifySSL = sslVerifyCheck->isChecked();
    ProxyManager proxy;

    if (useProxyCheck->isChecked()) {
        const QString proxyText = proxyInput->text();
        if (QFile::exists(proxyText)) {
            QFile file(proxyText);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                std::vector<std::string> proxies, liveProxies;
                while (!in.atEnd()) proxies.push_back(in.readLine().trimmed().toStdString());
                file.close();
                for (const auto &p: proxies) {
                    if (proxy.testProxy(p)) liveProxies.push_back(p);
                }
                proxy.setProxyList(liveProxies);
                log(QString("Loaded %1 live proxies from %2").arg(liveProxies.size()).arg(proxies.size()));
            }
        }
        else {
            proxy.setProxy(proxyText.toStdString());
        }
    }
    proxy.enable(useProxyCheck->isChecked());

    log(QString("Starting %1 attack on %2 with %3 threads")
        .arg(type.c_str(), url.c_str()).arg(threads));
    if (type == "HTTP Flood") {
        log(QString("Sending %1 requests per thread at %2 RPS").arg(requests).arg(rps));
    }
    if (proxy.isEnabled()) {
        log(QString("Using %1 proxies").arg(proxy.proxyCount()));
    }
    if (!customHeader.empty()) {
        log(QString("Using custom header: %1").arg(customHeader.c_str()));
    }

    Attacker::start(url, type, threads, requests, rps, customHeader, verifySSL, proxy);
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
    }
    else {
        log("Attack resumed.");
        pauseButton->setText("Pause");
    }
}

void DDoSSimulator::clearLog() const {
    logDisplay->clear();
}

void DDoSSimulator::loadProxyFile() {
    const QString fileName = QFileDialog::getOpenFileName(this, "Load Proxy List", "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        proxyInput->setText(fileName);
        log(QString("Selected proxy file: %1").arg(fileName));
    }
}

void DDoSSimulator::updateStatus() const {
    if (Attacker::isAttacking()) {
        log(QString("Status: Successful requests: %1 | Failed requests: %2")
            .arg(Attacker::getSuccessfulRequests())
            .arg(Attacker::getFailedRequests()));
    }
}

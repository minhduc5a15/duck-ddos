#include "simulator.h"
#include "attacker.h"
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
    setMinimumSize(600, 400);

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

    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    pauseButton = new QPushButton("Pause", this);
    clearButton = new QPushButton("Clear Log", this);

    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);

    auto* layout = new QGridLayout;
    layout->addWidget(new QLabel("Target URL:"), 0, 0);
    layout->addWidget(urlInput, 0, 1, 1, 2);
    layout->addWidget(new QLabel("Attack Type:"), 1, 0);
    layout->addWidget(attackType, 1, 1, 1, 2);
    layout->addWidget(new QLabel("Threads:"), 2, 0);
    layout->addWidget(threadSlider, 2, 1, 1, 2);
    layout->addWidget(new QLabel("Requests (HTTP Flood):"), 3, 0);
    layout->addWidget(requestCount, 3, 1, 1, 2);
    layout->addWidget(startButton, 4, 0);
    layout->addWidget(stopButton, 4, 1);
    layout->addWidget(pauseButton, 4, 2);
    layout->addWidget(clearButton, 4, 3);
    layout->addWidget(logDisplay, 5, 0, 1, 4);
    setLayout(layout);

    connect(startButton, &QPushButton::clicked, this, &DDoSSimulator::startAttack);
    connect(stopButton, &QPushButton::clicked, this, &DDoSSimulator::stopAttack);
    connect(pauseButton, &QPushButton::clicked, this, &DDoSSimulator::pauseAttack);
    connect(clearButton, &QPushButton::clicked, this, &DDoSSimulator::clearLog);
}

void DDoSSimulator::applyStyle() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b; /* Xám đậm tối làm nền */
            color: #d4d4d4; /* Chữ xám sáng nhẹ */
            font-family: "Arial", sans-serif; /* Giữ font Arial */
        }
        QLineEdit, QComboBox, QSpinBox {
            background-color: #3c3c3c; /* Xám tối nhạt */
            border: 1px solid #555555; /* Viền xám trung */
            border-radius: 5px; /* Bo tròn 5px */
            padding: 5px;
            color: #d4d4d4; /* Chữ xám sáng */
        }
        QPushButton {
            background-color: #5c85d6; /* Xanh dương đậm nhẹ */
            color: #ffffff; /* Chữ trắng */
            border: none;
            padding: 8px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #476bb3; /* Xanh đậm hơn khi hover */
        }
        QTextEdit {
            background-color: #1e1e1e; /* Đen nhạt cho log */
            border: 1px solid #555555; /* Viền xám trung */
            border-radius: 5px; /* Bo tròn 5px */
            color: #d4d4d4; /* Chữ xám sáng */
        }
        QSlider::groove:horizontal {
            background: #555555; /* Thanh slider xám trung */
            height: 8px;
        }
        QSlider::handle:horizontal {
            background: #5c85d6; /* Tay cầm xanh dương */
            width: 16px;
        }
    )");
}

void DDoSSimulator::log(const QString& message) const {
    const QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    logDisplay->append(timestamp + message);
}

bool DDoSSimulator::validateUrl(const QString& url) const {
    static const QRegularExpression re("^(http|https)://[a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,}(/.*)?$");
    return re.match(url).hasMatch();
}

void DDoSSimulator::startAttack() {
    if (urlInput->text().isEmpty() || !validateUrl(urlInput->text())) {
        QMessageBox::warning(this, "Error", "Please enter a valid URL (e.g., http://example.com)!");
        return;
    }

    const std::string url = urlInput->text().toStdString();
    const std::string type = attackType->currentText().toStdString();
    const int threads = threadSlider->value();
    const int requests = requestCount->value();

    log(QString("Starting %1 attack on %2 with %3 threads")
        .arg(type.c_str(), url.c_str()).arg(threads));
    if (type == "HTTP Flood") {
        log(QString("Sending %1 requests per thread").arg(requests));
    }

    Attacker::start(url, type, threads, requests);
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
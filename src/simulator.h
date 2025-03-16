#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QSpinBox>
#include <QCheckBox>

class DDoSSimulator final : public QWidget {
    Q_OBJECT
public:
    explicit DDoSSimulator(QWidget* parent = nullptr);
    ~DDoSSimulator() override = default;

    private slots:
        void startAttack();
    void stopAttack() const;
    void pauseAttack();
    void clearLog() const;

private:
    QLineEdit* urlInput{};
    QComboBox* attackType{};
    QSlider* threadSlider{};
    QSpinBox* requestCount{};
    QLineEdit* proxyInput{};
    QCheckBox* useProxyCheck{};
    QPushButton* startButton{};
    QPushButton* stopButton{};
    QPushButton* pauseButton{};
    QPushButton* clearButton{};
    QTextEdit* logDisplay{};

    void setupUi();
    void applyStyle();
    void log(const QString& message) const;
    bool validateUrl(const QString& url) const;
    bool validateProxy(const QString& proxy) const;
};

#endif // SIMULATOR_H
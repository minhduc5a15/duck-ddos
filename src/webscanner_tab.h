#ifndef WEBSCANNER_TAB_H
#define WEBSCANNER_TAB_H

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QCheckBox>
#include <QTimer>

class WebScannerTab final : public QWidget {
    Q_OBJECT
public:
    explicit WebScannerTab(QWidget *parent = nullptr);

    private slots:
        void startScan();
    void stopScan() const;
    void updateScanResult() const;

private:
    QLineEdit *urlInput{};
    QComboBox *scanType{};
    QSlider *threadSlider{};
    QLineEdit *proxyInput{};
    QCheckBox *useProxyCheck{};
    QLineEdit *headerInput{};
    QCheckBox *sslVerifyCheck{};
    QPushButton *startButton{};
    QPushButton *stopButton{};
    QTextEdit *resultDisplay{};
    QTimer *resultTimer{};

    void setupUi();
    void log(const QString &message) const;
    bool validateUrl(const QString &url) const;
};

#endif // WEBSCANNER_TAB_H
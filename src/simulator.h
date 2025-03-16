#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer> // Thêm để cập nhật trạng thái

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
    void loadProxyFile(); // Tải proxy từ file
    void updateStatus() const; // Cập nhật trạng thái

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
    QPushButton* loadProxyButton{}; // Nút tải proxy
    QTextEdit* logDisplay{};
    QTimer* statusTimer{}; // Timer cập nhật trạng thái

    void setupUi();
    void applyStyle();
    void log(const QString& message) const;
    bool validateUrl(const QString& url) const;
    bool validateProxy(const QString& proxy) const;
};

#endif // SIMULATOR_H
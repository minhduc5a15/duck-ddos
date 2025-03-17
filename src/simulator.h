#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>

class DDoSSimulator final : public QWidget {
    Q_OBJECT

public:
    explicit DDoSSimulator(QWidget *parent = nullptr);

    ~DDoSSimulator() override = default;

private slots:
    void start_attack();

    void stop_attack() const;

    void pause_attack();

    void clear_log() const;

    void load_proxy_file();

    void update_status() const;

private:
    QLineEdit *url_input_{};
    QComboBox *attack_type_{};
    QSlider *thread_slider_{};
    QSpinBox *request_count_{};
    QSpinBox *rps_input_{};
    QLineEdit *proxy_input_{};
    QCheckBox *use_proxy_check_{};
    QLineEdit *header_input_{};
    QCheckBox *ssl_verify_check_{};
    QPushButton *start_button_{};
    QPushButton *stop_button_{};
    QPushButton *pause_button_{};
    QPushButton *clear_button_{};
    QPushButton *load_proxy_button_{};
    QTextEdit *log_display_{};
    QTimer *status_timer_{};

    void setup_ui();

    void log(const QString &message) const;

    bool validate_url(const QString &url) const;

    bool validate_proxy(const QString &proxy) const;
};

#endif // SIMULATOR_H

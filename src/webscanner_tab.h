#ifndef WEBSCANNER_TAB_H
#define WEBSCANNER_TAB_H

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QCheckBox>
#include <QTimer>
#include <QProgressBar>
#include <QFileDialog>

class WebScannerTab final : public QWidget {
    Q_OBJECT

public:
    explicit WebScannerTab(QWidget *parent = nullptr);

    private slots:
        void start_scan();
    void stop_scan() const;
    void update_scan_result() const;
    void export_result();

private:
    QLineEdit *url_input_{};
    QComboBox *scan_type_{};
    QSlider *thread_slider_{};
    QLineEdit *proxy_input_{};
    QCheckBox *use_proxy_check_{};
    QLineEdit *header_input_{};
    QCheckBox *ssl_verify_check_{};
    QPushButton *start_button_{};
    QPushButton *stop_button_{};
    QPushButton *export_button_{};
    QTextEdit *result_display_{};
    QTimer *result_timer_{};
    QProgressBar *progress_bar_{};
    QLineEdit *request_rate_input_{};
    QLineEdit *timeout_input_{};

    void setup_ui();
    void log(const QString &message) const;
    bool validate_url(const QString &url) const;
};

#endif // WEBSCANNER_TAB_H
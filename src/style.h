#ifndef STYLE_H
#define STYLE_H

#include <QString>

class Style {
public:
    static QString darkTheme() {
        return R"(
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
            QTabWidget::pane {
                border: 1px solid #555555;
                background: #2b2b2b;
            }
            QTabBar::tab {
                background: #3c3c3c;
                color: #d4d4d4;
                padding: 8px;
            }
            QTabBar::tab:selected {
                background: #5c85d6;
                color: #ffffff;
            }
        )";
    }
};

#endif // STYLE_H

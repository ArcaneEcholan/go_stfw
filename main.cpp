#include <QApplication>
#include <QMainWindow>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QHotkey>
#include "ui_main.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
    ui.setupUi(this);
    // Set up the global shortcut
    QHotkey* shortcut = new QHotkey(QKeySequence("Alt+Q"), true, this); // 'true' enables the hotkey
    connect(shortcut, &QHotkey::activated, this, &MainWindow::showInputDialog);
  }

private slots:
  void showInputDialog() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Search"),
                                         tr("Enter search query:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
      QString url = "https://www.google.com/search?q=" + text;
      QDesktopServices::openUrl(QUrl(url));
    }
  }

private:
  Ui::MainWindow ui;
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.show();
  return app.exec();
}

#include "main.moc"

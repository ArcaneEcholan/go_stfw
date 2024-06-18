#include <QApplication>
#include <QGuiApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QScreen>
#include <QScrollBar>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebEngineView>
#include <cmark.h>

#include <QFileDialog>
#include <iostream>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr)
      : QMainWindow(parent), debounceTimer(new QTimer(this)) {
    setWindowTitle("WX Markdown Editor");

    // Set initial size of the main window
    resize(1200, 800); // Width: 1200, Height: 800

    // Center the window on the screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    setGeometry(x, y, width(), height());

    // Create a central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create a vertical layout for the central widget
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Create a splitter to hold the text editor and the web view
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // Create a text editor (input editor) on the left
    textEditor = new QTextEdit(this);
    textEditor->setPlaceholderText("Enter Markdown or HTML text here...");

    // Create a web view on the right
    QWebEngineView *webView = new QWebEngineView(this);

    // Add the text editor and the web view to the splitter
    splitter->addWidget(textEditor);
    splitter->addWidget(webView);

    // Set the stretch factors to make them equally spaced
    splitter->setStretchFactor(0, 1); // Text editor
    splitter->setStretchFactor(1, 1); // Web view

    // Set initial sizes (optional, to enforce starting with half-half)
    QList<int> sizes;
    sizes << width() / 2 << width() / 2; // Half-half
    splitter->setSizes(sizes);

    // Set the splitter as the main layout
    layout->addWidget(splitter);

    // Connect text editor changes to update the web view
    connect(textEditor, &QTextEdit::textChanged, this, [this, webView]() {
      QString markdownContent = textEditor->toPlainText();
      QByteArray markdownBytes = markdownContent.toUtf8();
      const char *markdownText = markdownBytes.constData();

      // Convert Markdown to HTML using cmark
      cmark_node *doc = cmark_parse_document(markdownText, markdownBytes.size(),
                                             CMARK_OPT_DEFAULT);
      char *html = cmark_render_html(doc, CMARK_OPT_DEFAULT);
      cmark_node_free(doc);

      QString htmlContent = QString::fromUtf8(html);
      free(html);

      // Set the converted HTML to the web view
      webView->setHtml(htmlContent);
    });

    connect(textEditor->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this]() {
              // Restart the debounce timer
              if (!debounceTimer->isActive()) {
                std::cout << "scroll event" << std::endl;
                debounceTimer->start();
              }
            });

    // Setup debounce timer
    debounceTimer->setSingleShot(true);
    debounceTimer->setInterval(100); // Adjust the interval for debouncing

    // Connect text editor scroll changes to sync with the web view
    connect(textEditor->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this]() {
              // Restart the debounce timer
              if (!debounceTimer->isActive()) {
                std::cout << "scroll event" << std::endl;
                debounceTimer->start();
              }
            });

    // Connect the debounce timer's timeout signal to the function that updates
    // the web view
    connect(debounceTimer, &QTimer::timeout, this, [this, webView]() {
      std::cout << "scroll changed\n" << std::endl;
      int scrollPosition = textEditor->verticalScrollBar()->value();
      int maxScroll = textEditor->verticalScrollBar()->maximum();

      // Calculate the scroll percentage
      double scrollPercentage = static_cast<double>(scrollPosition) / maxScroll;

      // Convert the scroll percentage to the web view's scroll position
      QString jsCode = QString("window.scrollTo(0, (document.body.scrollHeight "
                               "- document.body.clientHeight) * %1);")
                           .arg(scrollPercentage);

      // Execute JavaScript in the web view to scroll
      webView->page()->runJavaScript(jsCode);
    });

    // Load some initial content into the web view
    webView->setHtml("<h1>Welcome to WX Markdown Viewer</h1>");

    createMenuBar();
  }

private:
  QTimer *debounceTimer; // Timer for debouncing

  QString currentFilePath;

  QTextEdit *textEditor;

  void createMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu("File");

    QAction *openAction = fileMenu->addAction("Open");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveAction = fileMenu->addAction("Save");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    QAction *saveAsAction = fileMenu->addAction("Save As");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    setMenuBar(menuBar);
  }

private slots:
   void openFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "Open File", "", "Text Files (*.txt *.md);;All Files (*)");
    if (!filePath.isEmpty()) {
      currentFilePath = filePath; // Store the file path
      QFile file(filePath);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        textEditor->setPlainText(content);
        file.close();
      }
    }
  }

  void saveFile() {
    if (!currentFilePath.isEmpty()) {
      QFile file(currentFilePath);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << textEditor->toPlainText();
        file.close();
      }
    } else {
      saveFileAs(); // If no file path is set, trigger Save As
    }
  }

  void saveFileAs() {
    QString filePath = QFileDialog::getSaveFileName(
        this, "Save File As", "", "Text Files (*.txt *.md);;All Files (*)");
    if (!filePath.isEmpty()) {
      currentFilePath = filePath; // Update the file path
      saveFile();                 // Save to the new file path
    }
  }
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.show();
  return app.exec();
}

#include "main.moc"

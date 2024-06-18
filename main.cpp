#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QMainWindow>
#include <QMenuBar>
#include <QMimeData>
#include <QScreen>
#include <QScrollBar>
#include <QShortcut>
#include <QSplitter>
#include <QTextEdit>
#include <QTextStream>
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
      std::string s =  std::string("<html><head></head><body>") + html + "</body></html>";
       //std::string s =   std::string("<html><head></head><body>") + html + "<script>window.onload =()=> { alert(`${window.location.host}`)};</script></body></html>";

      cmark_node_free(doc);

      QString htmlContent = QString::fromUtf8(s.c_str());
      std::cout << "htmlContent: " << htmlContent.toStdString() << std::endl;
      free(html);

      // Determine the base URL for the web view
      // Must use QUrl object, not QString or other string.
      QUrl baseUrl = QUrl::fromLocalFile(QFileInfo(currentFilePath).absolutePath() + "/");

      std::cout << "qt webviewer baseurl: " << baseUrl.toDisplayString().toStdString() << std::endl;
      // Set the converted HTML to the web view
      webView->setHtml(htmlContent, baseUrl);
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

    // Create shortcut for Ctrl+Shift+V to paste image from clipboard
    QShortcut *pasteImageShortcut =
        new QShortcut(QKeySequence("Ctrl+Shift+V"), this);
    connect(pasteImageShortcut, &QShortcut::activated, this,
            &MainWindow::pasteImageFromClipboard);
  }

private:
  QTimer *debounceTimer; // Timer for debouncing

  QString currentFilePath;

  QTextEdit *textEditor;

  QString generateUniqueImagePath() {
    QDir dir(QFileInfo(currentFilePath).absolutePath());
    QString baseName = QFileInfo(currentFilePath).completeBaseName();
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString imageName = QString("%1_image_%2.png").arg(baseName).arg(timestamp);
    return dir.filePath(imageName);
  }

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
  void pasteImageFromClipboard() {
    std::cout << "ctl+shift+v" << std::endl;
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    // the image data obtain process is learned from:
    // https://stackoverflow.com/questions/46825795/how-to-get-clipboard-image-from-qclipboard-mimedata-in-c
    if (mimeData->hasImage()) {
      std::cout << "clipboard has image" << std::endl;
      QImage image = clipboard->image();

      // QImage image = qvariant_cast<QImage>(mimeData->imageData());
      if (!image.isNull() && !currentFilePath.isEmpty()) {
        // Generate a unique file name based on current time
        QString imagePath = generateUniqueImagePath();

        std::cout << "Image path: " << imagePath.toStdString() << std::endl;

        // Save the image to the file
        if (image.save(imagePath)) {
          // Insert Markdown image link at the cursor position
          QString imageMarkdown =
              QString("![Image](%1)")
                  .arg(QFileInfo(imagePath).absoluteFilePath());
          textEditor->insertPlainText(imageMarkdown);
        } else {
          std::cerr << "Failed to save image to file: "
                    << imagePath.toStdString() << std::endl;
        }
      }
    }
  }

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

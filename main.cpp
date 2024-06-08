#include "ui_main.h"
#include <QApplication>
#include <QDesktopServices>
#include <QHotkey>
#include <QInputDialog>
#include <QListView>
#include <QListWidget>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QUrl>
#include <QVBoxLayout> // For layout management
#include <QWebEngineView>

#include <QDateTime>
#include <QString>

#include <QPainter>
#include <QStyledItemDelegate>

#include "filesystem"
#include "iostream"
int count =  0;
class DocumentDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  DocumentDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    // Retrieve the document data
    QString name = index.model()->data(index.siblingAtColumn(0)).toString();
    QString created = index.model()->data(index.siblingAtColumn(1)).toString();
    QString modified = index.model()->data(index.siblingAtColumn(2)).toString();

    // Draw the background
    painter->save();
    painter->setClipRect(option.rect);
    painter->fillRect(option.rect, option.state & QStyle::State_Selected
                                       ? option.palette.highlight()
                                       : option.palette.base());
    painter->restore();

    // Draw the document name
    QRect nameRect = option.rect.adjusted(5, 5, -5, -option.rect.height() / 2);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);

    // Draw the creation and modification times
    QRect timeRect = option.rect.adjusted(5, option.rect.height() / 2, -5, -5);
    painter->drawText(
        timeRect, Qt::AlignLeft | Qt::AlignVCenter,
        QString("Created: %1, Modified: %2").arg(created).arg(modified));
  }

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override {
    // Provide a size hint for each item
    //return QSize(option.rect.width(), 100);
    std::cout << option.rect.height() << std::endl;
    std::cout << count ++ << std::endl;

    return QSize(option.rect.width(), 40);
  }
};

class Document {
public:
  Document(const QString &name, const QDateTime &created,
           const QDateTime &modified)
      : name(name), created(created), modified(modified) {}

  QString getName() const { return name; }
  QDateTime getCreated() const { return created; }
  QDateTime getModified() const { return modified; }

private:
  QString name;
  QDateTime created;
  QDateTime modified;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr)
      : QMainWindow(parent), listView(new QListView(this)),
        model(new QStandardItemModel(this)) {
    setWindowTitle("WX Markdown");
    // ui.setupUi(this);
    //// Set up the global shortcut
    // QHotkey* shortcut = new QHotkey(QKeySequence("Alt+Q"), true, this); //
    // 'true' enables the hotkey connect(shortcut, &QHotkey::activated, this,
    // &MainWindow::showInputDialog);

    // Set the central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    QFont font = listView->font();
    //font.setFamily("Courier New"); // A monospaced font that clearly displays underscores
    font.setPointSize(10); // Adjust size as needed, if don't, the underscore in the file's name will not be displayed
    listView->setFont(font);

    // Configure the QListView
    listView->setModel(model);
    listView->setItemDelegate(new DocumentDelegate(this)); // Set the custom delegate

    // Disable item editing
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Add the QListView to the layout
    layout->addWidget(listView);

    // Set the central widget for the main window
    setCentralWidget(centralWidget);

    // Populate the list with document data
    populateList();
  }

private:
  QListView *listView;       // Pointer to the QListView
  QStandardItemModel *model; // Model to hold the document data

  QListWidget *listWidget; // Pointer to the QListWidget

  void showInputDialog() {
    bool ok;
    QString text =
        QInputDialog::getText(this, tr("Search"), tr("Enter search query:"),
                              QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
      QString url = "https://www.google.com/search?q=" + text;
      QDesktopServices::openUrl(QUrl(url));
    }
  }

  void populateList() {
    QList<Document> documents{};
    std::string path = "/home/chaowen/temp";
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      documents.append(
          Document(QString::fromStdString(entry.path().filename().string()),
                   QDateTime::currentDateTime().addDays(-2),
                   QDateTime::currentDateTime().addDays(-1)));
    }
    //
    //
    //// List of documents to add
    // QList<Document> documents = {
    //     Document("Document 1", QDateTime::currentDateTime().addDays(-2),
    //              QDateTime::currentDateTime().addDays(-1)),
    //     Document("Document 2", QDateTime::currentDateTime().addDays(-5),
    //              QDateTime::currentDateTime().addDays(-3)),
    //     Document("Document 3", QDateTime::currentDateTime().addDays(-10),
    //              QDateTime::currentDateTime())};

    // Set the model columns
    model->setHorizontalHeaderLabels({"Name", "Created", "Modified"});

    // Add documents to the model
    foreach (const Document &doc, documents) {
      QList<QStandardItem *> items;
      items.append(new QStandardItem(doc.getName()));
      items.append(
          new QStandardItem(doc.getCreated().toString("yyyy-MM-dd HH:mm:ss")));
      items.append(
          new QStandardItem(doc.getModified().toString("yyyy-MM-dd HH:mm:ss")));
      model->appendRow(items);
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

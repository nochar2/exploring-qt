#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    CustomTreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent) {}

protected:
    // XXX: This completely breaks the > icon, and double click is still consumed
    // as single click. Thanks, Claude.
    void mousePressEvent(QMouseEvent *event) override
    {
        QTreeWidgetItem *item = itemAt(event->pos());
        if (item) {
            // Toggle expansion state on single click
            if (item->childCount() > 0) {
                item->setExpanded(!item->isExpanded());
            }
        }
        
        // Call parent implementation for selection handling
        QTreeWidget::mousePressEvent(event);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupMenus();
        setupConnections();
    }

private slots:
    void onItemSelectionChanged()
    {
        QTreeWidgetItem *current = treeWidget->currentItem();
        if (current) {
            fileLabel->setText(current->text(0));
        } else {
            fileLabel->setText("No file selected");
        }
    }

    void showHelp()
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Help");
        msgBox.setText("Help information will go here.");
        msgBox.exec();
    }

private:
    CustomTreeWidget *treeWidget;
    QLabel *fileLabel;

    void setupUI()
    {
        // Create central widget and main layout
        QWidget *centralWidget = new QWidget();
        setCentralWidget(centralWidget);
        
        QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
        
        // Create tree widget (left third)
        treeWidget = new CustomTreeWidget();
        
        // Set column headers (only Name and Type)
        QStringList headers;
        headers << "Name" << "Type";
        treeWidget->setHeaderLabels(headers);
        
        // Create minimal dummy data
        QTreeWidgetItem *documentsItem = new QTreeWidgetItem(treeWidget);
        documentsItem->setText(0, "Documents");
        documentsItem->setText(1, "Folder");
        
        QTreeWidgetItem *docChild1 = new QTreeWidgetItem(documentsItem);
        docChild1->setText(0, "report.pdf");
        docChild1->setText(1, "PDF Document");
        
        QTreeWidgetItem *picturesItem = new QTreeWidgetItem(treeWidget);
        picturesItem->setText(0, "Pictures");
        picturesItem->setText(1, "Folder");
        
        QTreeWidgetItem *musicItem = new QTreeWidgetItem(treeWidget);
        musicItem->setText(0, "music.mp3");
        musicItem->setText(1, "MP3 Audio");
        
        // Configure tree widget
        treeWidget->setExpandsOnDoubleClick(false);
        treeWidget->expandAll();
        treeWidget->resizeColumnToContents(0);
        treeWidget->setAlternatingRowColors(true);
        
        // Create file label (right two thirds)
        fileLabel = new QLabel("No file selected");
        fileLabel->setAlignment(Qt::AlignCenter);
        fileLabel->setStyleSheet("font-size: 16px; color: #666;");
        
        // Add widgets to layout with stretch factors
        mainLayout->addWidget(treeWidget, 1);      // Left third
        mainLayout->addWidget(fileLabel, 2);       // Right two thirds
        
        // Set window properties
        setWindowTitle("File Explorer");
        resize(800, 500);
    }
    
    void setupMenus()
    {
        QMenuBar *menuBar = this->menuBar();
        
        // File menu
        QMenu *fileMenu = menuBar->addMenu("File");
        QAction *exitAction = fileMenu->addAction("Exit");
        connect(exitAction, &QAction::triggered, this, &QWidget::close);
        
        // Help menu
        QMenu *helpMenu = menuBar->addMenu("Help");
        QAction *helpAction = helpMenu->addAction("About");
        connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    }
    
    void setupConnections()
    {
        connect(treeWidget, &QTreeWidget::itemSelectionChanged, 
                this, &MainWindow::onItemSelectionChanged);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "claude.moc"  // Required for Q_OBJECT in .cpp file

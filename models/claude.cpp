#include <QtWidgets>
#include <QApplication>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QHeaderView>

// edited a bit

enum class Priority { Low, Medium, High };
QStringList priority_strings() { return {"Low", "Medium", "High"}; }

struct DataItem {
    QString key;
    Priority priority;
};

template<typename F, typename T>
T convert_from_to(F from);

QString convert_from_to(Priority p) {
    using enum Priority;
    switch(p) {
        case Low:    return "Low";
        case Medium: return "Medium";
        case High:   return "High";
    }
    throw std::invalid_argument("non-exhaustive switch");
};

Priority convert_from_to(const QString &str) {
    using enum Priority;
    if (str == "Low")    return Low;
    if (str == "Medium") return Medium;
    if (str == "High")   return High;
    throw std::invalid_argument(std::format("can't convert string {} to priority\n", str.toStdString()));
}


// Custom delegate for dropdown in value column
class ComboBoxDelegate : public QStyledItemDelegate {
public:
    ComboBoxDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, 
                         const QModelIndex& index) const override {
        if (index.column() == 1) { // Value column
            QComboBox* combo = new QComboBox(parent);
            combo->addItems(priority_strings());
            return combo;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (index.column() == 1) {
            QComboBox* combo = qobject_cast<QComboBox*>(editor);
            assert(combo);
            QString value = index.model()->data(index, Qt::EditRole).toString();
            combo->setCurrentText(value);
        } else {
            QStyledItemDelegate::setEditorData(editor, index);
        }
    }
    
    void setModelData(QWidget* editor, QAbstractItemModel* model, 
                     const QModelIndex& index) const override {
        if (index.column() == 1) {
            QComboBox* combo = qobject_cast<QComboBox*>(editor);
            assert(combo);
            model->setData(index, combo->currentText(), Qt::EditRole);
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }
};

// Custom model that manages your internal data
class KVTreeModel : public QStandardItemModel {
    Q_OBJECT
    
private:
    QList<DataItem> m_data;
    
public:
    KVTreeModel(QObject* parent = nullptr) : QStandardItemModel(parent) {
        setHorizontalHeaderLabels({"Key", "Value"});
        
        // Initialize with some sample data
        
        m_data.append({"Priority", Priority::Medium});
        m_data.append({"Status", Priority::High}); // Using Priority enum for demo
        
        refreshModel();
    }
    
    void refreshModel() {
        // clear();
        // setHorizontalHeaderLabels({"Key", "Value"});
        
        for (int i = 0; i < m_data.size(); ++i) {
            QStandardItem* keyItem = new QStandardItem(m_data[i].key);
            keyItem->setEditable(false); // Make key read-only
            
            QString pri = convert_from_to<Priority, QString>(m_data[i].priority);
            QStandardItem* valueItem = new QStandardItem(pri);
            
            appendRow({keyItem, valueItem});
        }
    }
    
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override {
        if (role == Qt::EditRole && index.column() == 1) {
            // Update internal data when value changes
            if (index.row() < m_data.size()) {
                QString newValue = value.toString();
                m_data[index.row()].priority = convert_from_to<const QString &, Priority>(newValue);
                
                // Emit signal that data changed
                emit dataChanged(index, index);
                emit priorityChanged(index.row(), m_data[index.row()].priority);
                
                return QStandardItemModel::setData(index, value, role);
            }
        }
        return QStandardItemModel::setData(index, value, role);
    }
    
    // Convenience methods to access your internal data
    Priority getPriority(int index) const {
        if (index < m_data.size()) {
            return m_data[index].priority;
        }
        return Priority::Low;
    }
    
    void setPriority(int index, Priority priority) {
        if (index < m_data.size()) {
            m_data[index].priority = priority;
            refreshModel();
        }
    }
    
signals:
    void priorityChanged(int index, Priority priority);
};

// Main widget
class TreeViewWidget : public QWidget {
    Q_OBJECT
    
private:
    QTreeView* m_treeView;
    KVTreeModel* m_model;
    ComboBoxDelegate* m_delegate;
    
public:
    TreeViewWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        m_treeView = new QTreeView(this);
        m_model = new KVTreeModel(this);
        m_delegate = new ComboBoxDelegate(this);
        
        m_treeView->setModel(m_model);
        m_treeView->setItemDelegate(m_delegate);
        
        // Configure tree view
        m_treeView->setRootIsDecorated(false); // No expand/collapse icons
        m_treeView->setAlternatingRowColors(true);
        m_treeView->header()->setStretchLastSection(true);
        m_treeView->header()->resizeSection(0, 150);
        
        layout->addWidget(m_treeView);

        connect(m_model, &KVTreeModel::priorityChanged,
        this, &TreeViewWidget::onPriorityChanged);

    }
    
private slots:
    void onPriorityChanged(int index, Priority priority) {
        qDebug() << "Priority changed for item" << index << "to" 
                 << convert_from_to<Priority, QString>(priority);
    }
};

// Main application
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    TreeViewWidget widget;
    widget.resize(400, 200);
    widget.show();
    
    return app.exec();
}

#include "claude.moc"

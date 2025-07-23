#include <QtWidgets>
#include <QApplication>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QHeaderView>

enum class Priority {
    Low,
    Medium,
    High
};

// Convert between enum and string
class PriorityHelper {
public:
    static QString enumToString(Priority p) {
        switch(p) {
            case Priority::Low: return "Low";
            case Priority::Medium: return "Medium";
            case Priority::High: return "High";
        }
        return "Low";
    }
    
    static Priority stringToEnum(const QString& str) {
        if (str == "Medium") return Priority::Medium;
        if (str == "High") return Priority::High;
        return Priority::Low;
    }
    
    static QStringList getAllOptions() {
        return {"Low", "Medium", "High"};
    }
};

enum class ValueType {
    PriorityEnum,
    ReadOnlyString,
    EditableString
    // Add more types as needed
};


using DataType = std::variant<QString,Priority>;
// this allows you to interop between std::variant and QVariant ????????
// Q_DECLARE_METATYPE(DataType);

struct DataItem {
    QString key;
    ValueType discr;
    DataType value;
};

class KVTreeModel : public QStandardItemModel {
    Q_OBJECT

public:
    QList<DataItem> m_data;

    KVTreeModel(QObject* parent = nullptr) : QStandardItemModel(parent) {
        setHorizontalHeaderLabels({"Key", "Value"});
        
        // Initialize with different types of data
        using enum ValueType;

        m_data.append({.key="Priority",    .discr=PriorityEnum,   .value=Priority::Medium});
        m_data.append({.key="Status",      .discr=ReadOnlyString, .value="Active (read-only)"});
        m_data.append({.key="Description", .discr=EditableString, .value="Edit me!"});
        
        refreshModel();
    }
    
    void refreshModel() {
        clear();
        setHorizontalHeaderLabels({"Key", "Value"});
        
        for (int i = 0; i < m_data.size(); ++i) {
            QStandardItem* keyItem = new QStandardItem(m_data[i].key);
            keyItem->setEditable(false); // Make key read-only
            
            QString displayValue;
            switch (m_data[i].discr) {
                case ValueType::PriorityEnum:
                    displayValue = PriorityHelper::enumToString(std::get<Priority>(m_data[i].value)); break;
                case ValueType::ReadOnlyString:
                case ValueType::EditableString:
                    displayValue = std::get<QString>(m_data[i].value); break;
            }
            
            QStandardItem* valueItem = new QStandardItem(displayValue);
            
            // Make read-only string items non-editable
            if (m_data[i].discr == ValueType::ReadOnlyString) {
                valueItem->setEditable(false);
                // Optional: change appearance for read-only items
                valueItem->setForeground(QBrush(QColor(100, 100, 100))); // Gray text
            }
            
            appendRow({keyItem, valueItem});
        }
    }
    
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override {
        if (role == Qt::EditRole && index.column() == 1) {
            // Update internal data when setting a new value
            if (index.row() < m_data.size()) {
                QString newValue = value.toString();
                
                switch (m_data[index.row()].discr) {
                    case ValueType::PriorityEnum: {
                        Priority newPriority = PriorityHelper::stringToEnum(newValue);
                        m_data[index.row()].value = newPriority;
                        emit priorityChanged(index.row(), newPriority);
                        break;
                    }
                    case ValueType::EditableString: {
                        m_data[index.row()].value = newValue;
                        emit stringChanged(index.row(), newValue);
                        break;
                    }
                    case ValueType::ReadOnlyString:
                        // Should not happen, but just in case
                        return false;
                }
                
                // Emit signal that data changed
                emit dataChanged(index, index);
                return QStandardItemModel::setData(index, value, role);
            }
        }
        return QStandardItemModel::setData(index, value, role);
    }

    // -- fuck off with this OOP shit
    
    // Convenience methods to access your internal data
    // Priority getPriority(int index) const {
    //     if (index < m_data.size() && m_data[index].discr == ValueType::PriorityEnum) {
    //         return m_data[index].getPriority();
    //     }
    //     return Priority::Low;
    // }
    
    // QString getString(int index) const {
    //     if (index < m_data.size()) {
    //         return m_data[index].getString();
    //     }
    //     return QString();
    // }
    
    // void setPriority(int index, Priority priority) {
    //     if (index < m_data.size() && m_data[index].discr == ValueType::PriorityEnum) {
    //         m_data[index].value = static_cast<int>(priority);
    //         refreshModel();
    //     }
    // }
    
    // void setString(int index, const QString& str) {
    //     if (index < m_data.size() && m_data[index].discr == ValueType::EditableString) {
    //         m_data[index].value = str;
    //         refreshModel();
    //     }
    // }
    
signals:
    void priorityChanged(int index, Priority priority);
    void stringChanged(int index, const QString& value);
};// Your internal data model




// Custom delegate for different value types
class KVTreeViewDelegate : public QStyledItemDelegate {
private:
    KVTreeModel* m_model;
    
public:
    KVTreeViewDelegate(KVTreeModel* model, QObject* parent = nullptr) 
        : QStyledItemDelegate(parent), m_model(model) {}
    
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, 
                         const QModelIndex& index) const override {
        if (index.column() == 1) { // Value column
            ValueType type = m_model->m_data[index.row()].discr;
            
            switch (type) {
                case ValueType::PriorityEnum: {
                    QComboBox* combo = new QComboBox(parent);
                    combo->addItems(PriorityHelper::getAllOptions());
                    return combo;
                }
                case ValueType::EditableString: {
                    QLineEdit* lineEdit = new QLineEdit(parent);
                    return lineEdit;
                }
                case ValueType::ReadOnlyString:
                    // Return nullptr to prevent editing
                    return nullptr;
            }
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    
};

// Custom model that manages your internal data
;

// Main widget
class TreeViewWidget : public QWidget {
    Q_OBJECT
    
private:
    QTreeView* m_treeView;
    KVTreeModel* m_model;
    KVTreeViewDelegate* m_delegate;
    
public:
    TreeViewWidget(QWidget* parent = nullptr) : QWidget(parent) {
        // setup ui
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        m_treeView = new QTreeView(this);
        m_model    = new KVTreeModel(this);
        m_delegate = new KVTreeViewDelegate(m_model, this);
        
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
        connect(m_model, &KVTreeModel::stringChanged,
                this, &TreeViewWidget::onStringChanged);
    }
    
private:
    
private slots:
    void onPriorityChanged(int index, Priority priority) {
        qDebug() << "Priority changed for item" << index << "to" 
                 << PriorityHelper::enumToString(priority);
    }
    
    void onStringChanged(int index, const QString& value) {
        qDebug() << "String changed for item" << index << "to" << value;
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

#include "claude2.moc"

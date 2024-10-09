#include <QObject>
#include <QListWidget>
#include <QComboBox>

class ListSyncManager : public QObject
{
    Q_OBJECT

public:
    ListSyncManager(QListWidget *list1, QListWidget *list2, QComboBox *comboBox, QObject *parent = nullptr)
        : QObject(parent), list1_(list1), list2_(list2), comboBox_(comboBox)
    {
        connect(list1_, &QListWidget::itemChanged, this, &ListSyncManager::syncLists);
        connect(list1_, &QListWidget::itemActivated, this, &ListSyncManager::syncLists);
        connect(list1_, &QListWidget::itemClicked, this, &ListSyncManager::syncLists);
        connect(list1_, &QListWidget::itemDoubleClicked, this, &ListSyncManager::syncLists);

        syncLists();
    }

public slots:
    void syncLists()
    {
        list2_->clear();
        comboBox_->clear();
        comboBox_->addItem("Выбрать множество");

        for (int i = 0; i < list1_->count(); ++i)
        {
            QListWidgetItem *item = list1_->item(i)->clone();
            list2_->addItem(item);
            comboBox_->addItem(item->text());
        }
    }

private:
    QListWidget *list1_;
    QListWidget *list2_;
    QComboBox *comboBox_;
};

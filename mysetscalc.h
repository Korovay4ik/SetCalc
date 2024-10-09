#ifndef MYSETSCALC_H
#define MYSETSCALC_H

#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#include <algorithm>
#include <QStandardItemModel>
#include <QContextMenuEvent>
#include <QListWidgetItem>
#include <QtGlobal>
#include <QRandomGenerator>
#include <QStack>
#include <QChar>

#include "myset.h"
#include "ListSyncManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MySetsCalc; }
QT_END_NAMESPACE

class MySetsCalc : public QMainWindow
{
    Q_OBJECT

public:
    MySetsCalc(QWidget *parent = nullptr);
    ~MySetsCalc();

private slots:
    void on_pushButton_addSet_clicked();

    void on_lineEdit_elementsOfSet_textChanged(const QString &text);

    void on_pushButton_applyChanges_clicked();

    void on_pushButton_cancelChanges_clicked();

    void on_pushButton_randomizeSet_clicked();

    void on_checkBox_onlyPositiveNumber_stateChanged(int arg1);

    void on_checkBox_onlyNegativeNumber_stateChanged(int arg1);

    void on_pushButton_printAssociation_clicked();

    void on_pushButton_printIntersection_clicked();

    void on_pushButton_printDifference_clicked();

    void on_pushButton_printSymmetricalDifference_clicked();

    void on_pushButton_printAddition_clicked();

    void on_listWidget_setsToChoose_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_calculateFormul_clicked();

    void on_comboBox_newOrEdit_currentIndexChanged(int index);

    void on_comboBox_chooseSetForCalc_currentIndexChanged(const QString &arg1);

    void on_pushButton_clear_clicked();

    void on_pushButton_openBracket_clicked();

    void on_pushButton_closeBracket_clicked();

private:
    Ui::MySetsCalc *ui;
    std::vector<MySet*> allSets;
    QString defaultStyleSheet;
    QString defaultStyleSheetComboBox;
    QFont defaultFont;
    ListSyncManager *syncManager;

    void errorMessage(QString& text);

    void createContextMenu(QListWidget* listWidget);

    bool isDataCorrect(QString &setName, int &countElement, int &downLimit, int &upperLimit, std::vector<int> &set);

    void calculateFormul(QStringList formula);
};
#endif // MYSETSCALC_H

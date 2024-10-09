#include "mysetscalc.h"
#include "ui_mysetscalc.h"

MySetsCalc::MySetsCalc(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MySetsCalc)
{
    ui->setupUi(this);

    ui->lineEdit_formula->setAlignment(Qt::AlignCenter);

    ui->label_setsToChoose->setAlignment(Qt::AlignCenter);

    ui->label_sets->setAlignment(Qt::AlignCenter);
    ui->label_sets->hide();
    ui->listWidget_allSets->hide();
    ui->comboBox_chooseSetForCalc->hide();

    ui->lineEdit_calcResult->setReadOnly(true);
    ui->lineEdit_oldSetElement->setReadOnly(true);

    defaultFont = ui->lineEdit_calcResult->font();

    createContextMenu(ui->listWidget_allSets);

    defaultStyleSheet = ui->lineEdit_countElement->styleSheet();
    defaultStyleSheetComboBox = ui->comboBox_chooseSetForCalc->styleSheet();

    syncManager = new ListSyncManager(ui->listWidget_allSets, ui->listWidget_setsToChoose,
                                      ui->comboBox_chooseSetForCalc, this);
}

MySetsCalc::~MySetsCalc()
{
    delete ui;
}


bool isStringSetToVector(const QString& stringSet, std::vector<int>& set)
{
    if (stringSet.isEmpty())
        return true;
    QStringList stringSetList = stringSet.split(", ");
    for (auto &element : stringSetList)
    {
        bool ok = true;
        if (std::find(set.begin(), set.end(), element.toInt(&ok)) != set.end()){
            if (!ok){
                set.clear();
                return false;
            }
        }
        else{
            set.push_back(element.toInt(&ok));
            if (!ok){
                set.clear();
                return false;
            }
        }
    }
    return true;
}


QString vectToString(const std::vector<int> set)
{
    QString text = "";
    if (set.size() > 0){
        text += QString::number(set[0]);
    }
    for (int element = 1; element < int(set.size()); element++){
        text += ", " + QString::number(set[element]);
    }
    return text;
}


bool isForbiddenSymbol(QString text)
{
    QString forbuddenSymbols = " ()∪∩\\∆¬";
    for (auto& symbol : text){
        for (auto& forbiddenSymbol : forbuddenSymbols){
            if (symbol == forbiddenSymbol){
                return true;
            }
        }
    }
    return false;
}


bool isNameEngaged(QString setName, std::vector<MySet*> &sets)
{
    for (auto &set : sets){
        if (setName == set->getName()){
            return true;
        }
    }
    return false;
}


bool isOperation(const QString& token)
{
    return token == "∪" || token == "∩" || token == "\\" || token == "∆" || token == "¬";
}


bool isContainElement(int elementToFind, std::vector<int> set)
{
    for (auto &element : set){
        if (element == elementToFind){
            return true;
        }
    }
    return false;
}


bool isExpansionCorrect(QList<QString>& expansion)
{
    if (expansion.size()==1 && isOperation(expansion[0]))
        return false;
    for (int index = 0; index < expansion.size()-1; index++){
        if ((isOperation(expansion[index]) || expansion[index] == "¬") && isOperation(expansion[index+1]) && expansion[index+1] != "¬"){
            return false;
        }
    }
    return true;
}


bool isUniverseCorrect(QList<QString>& expansion, std::vector<MySet*> sets)
{
    int downLimit = 0, upperLimit = -1;
    for (auto& element : expansion){
        if (isNameEngaged(element, sets)){
            auto it = std::find_if(sets.begin(), sets.end(),
                         [element](MySet* set){return set->getName() == element;});
            MySet* foundSet = *it;
            if (downLimit == 0 && upperLimit == -1){
                downLimit = foundSet->getDownLim();
                upperLimit = foundSet->getUpperLim();
            }
            if (foundSet->getDownLim() != downLimit || foundSet->getUpperLim() != upperLimit){
                return false;
            }
        }
    }
    return true;
}


int findStartAddition(QString text)
{
    if (text.isEmpty())
        return 0;

    QStack<int> bkt;
    if (text.endsWith(')') || text.endsWith(") ")){
        int k = 1, index;
        if (text.endsWith(") ")){
            k = 2;
        }
        bkt.push(text.size()-k);
        for (index = text.size()-k-1; index > -1; index--){
            if (bkt.empty()){
                break;
            }
            if (text[index] == ')'){
                bkt.push(index);
            }
            else if (text[index] == '('){
                if (!bkt.isEmpty()){
                    bkt.pop();
                }
                else {
                    return index;
                }
            }
        }
        return index;
    }
    else {
        int index = text.size()-1;
        if (text[index] == ' '){
            index--;
        }
        while(index > 0){
            if (text[index] == ' ' || text[index] == '('){
                return index;
            }
            else if (!index){
                return 0;
            }
            index--;
        }
    }
    return 0;
}


QString makeStringSignInFormul(QString text, QString sign)
{
    QString resultText = text;
    if (text.isEmpty()){
        return sign + ' ';
    }
    if (text.endsWith(' ') || text.endsWith('(')){
        resultText += sign + ' ';
    }
    else {
        resultText += ' ' + sign + ' ';
    }
    return resultText;
}


void printStringSign(QLineEdit* lineEdit, QString sign)
{
    QString currentText = lineEdit->text();
    int cursorCurrentPos = lineEdit->cursorPosition();

    if (!isOperation(sign)){
        currentText = currentText.left(cursorCurrentPos) + sign +
                currentText.right(currentText.size() - cursorCurrentPos);
    }
    else{
        currentText = makeStringSignInFormul(currentText.left(cursorCurrentPos), sign) +
                currentText.right(currentText.size() - cursorCurrentPos);
    }

    lineEdit->setText(currentText);
}


bool isBktCorrect(QStringList formula)
{
    QStack<QChar> bkt;

    for (auto &element : formula){
        for (auto &symbol : element){
            if (symbol == '('){
                bkt.push(symbol);
            }
            else if (symbol == ')'){
                if (bkt.isEmpty()){
                    return false;
                }
                bkt.pop();
            }
        }
    }
    return bkt.isEmpty();
}


int findRequiredBracket(const QList<QString>& expression, int indexBracket)
{
    QStack<QString> bkt;
    if (expression[0] != "("){
        return -1;
    }
    bkt.push(expression[0]);
    for (int i = 1; i < expression.size(); i++){
        if (expression[i] == "("){
            bkt.push(expression[i]);
        }
        else if (expression[i] == ")"){
            bkt.pop();
        }
        if (bkt.isEmpty()){
            return indexBracket+i;
        }
    }
    return -1;
}


QList<QString> tokenize(const QString& expression) {
    QList<QString> tokens;
    QString currentToken;

    for (int i = 0; i < expression.size(); ++i) {
        QChar ch = expression[i];

        if (ch.isLetterOrNumber()) {
            // Начался новый операнд (множество)
            currentToken += ch;
        } else if (ch == QChar(0x222A) || ch == QChar(0x2229) || ch == '\\' || ch == QChar(0x2206) || ch == QChar(0x00AC)) {
            // Операция
            if (!currentToken.isEmpty()) {
                tokens.append(currentToken);
                currentToken.clear();
            }
            tokens.append(ch);
        } else if (ch == '(' || ch == ')') {
            // Скобка
            if (!currentToken.isEmpty()) {
                tokens.append(currentToken);
                currentToken.clear();
            }
            tokens.append(ch);
        }
    }

    if (!currentToken.isEmpty()) {
        tokens.append(currentToken);
    }

    return tokens;
}


bool isElementInList(const QString& textToFind, const QListWidget* listWidget)
{
    QListWidgetItem* item = nullptr;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* currentItem = listWidget->item(i);
        if (currentItem && currentItem->text() == textToFind) {
            item = currentItem;
            break;
        }
    }
    if (item)
        return true;
    return false;
}


struct TreeNode {
    QString value;
    TreeNode *left;
    TreeNode *right;

    TreeNode(QString val) : value(val), left(nullptr), right(nullptr) {}
};


TreeNode* buildExpressionTree(const QList<QString>& tokens, const QListWidget* operandList, int& tokenIndex) {
    QStack<TreeNode*> currentOperation;
    for (; tokenIndex < tokens.size(); tokenIndex++)
    {
        if (isElementInList(tokens[tokenIndex], operandList)){
            if (tokenIndex > 0){
                if (tokens[tokenIndex-1] == "¬"){
                    return new TreeNode(tokens[tokenIndex]);
                }
            }
            currentOperation.push(new TreeNode(tokens[tokenIndex]));
        }
        else if (isOperation(tokens[tokenIndex])){
            if (tokens[tokenIndex] != "¬"){
                currentOperation.push(new TreeNode(tokens[tokenIndex]));
            }
            else{
                TreeNode* operationAddition = new TreeNode(tokens[tokenIndex]);
                operationAddition->right = buildExpressionTree(tokens, operandList, ++tokenIndex);
                currentOperation.push(operationAddition);
            }
        }
        else if (tokens[tokenIndex] == "("){
            if (tokenIndex > 0 && tokens[tokenIndex-1] == "¬"){
                tokenIndex++;
                return buildExpressionTree(tokens, operandList, tokenIndex);
            }
            currentOperation.push(buildExpressionTree(tokens, operandList, ++tokenIndex));
        }
        else if (tokens[tokenIndex] == ")"){
            return currentOperation.pop();
        }
        else{
            return nullptr;
        }
        if (currentOperation.size() == 3){
            TreeNode* right = currentOperation.pop();
            TreeNode* operation = currentOperation.pop();
            TreeNode* left = currentOperation.pop();
            operation->left = left;
            operation->right = right;
            currentOperation.push(operation);
        }
    }

    if (currentOperation.size() != 1){
        return nullptr;
    }
    return currentOperation.pop();
}


MySet evaluateExpressionTree(TreeNode* root, std::vector<MySet*> sets)
{
    if (!root){
        return MySet();
    }

    // узел - это множество
    for (int indexSet = 0; indexSet < int(sets.size()); indexSet++){
        if (sets[indexSet]->getName() == root->value){
            return *sets[indexSet];
        }
    }

    // узел - это операция
    MySet leftSet = evaluateExpressionTree(root->left, sets);
    MySet rightSet = evaluateExpressionTree(root->right, sets);

    if (root->value == "∪") { // Объединение
        leftSet.countAssociation(rightSet);
    } else if (root->value == "∩") { // Пересечение
        leftSet.countInersection(rightSet);
    } else if (root->value == "\\") { // Разность
        leftSet.countDifference(rightSet);
    } else if (root->value == "∆") { // Симметрическая разность
        leftSet.countSymmetricalDifference(rightSet);
    } else if (root->value == "¬") { // Дополнение
        rightSet.countAddition();
        leftSet = rightSet;
    }

    return leftSet;
}


std::vector<int> generateRandomNumbers(int lowerBound, int upperBound, int count, int divisor) {
    if (!count)
        return {};
    std::vector<int> result;
    int countOfCorrectNumber = 0;

    srand(time(0));

    for (int element = lowerBound; element < upperBound+1; element++){
        if (element % divisor == 0){
            countOfCorrectNumber++;
        }
        if (countOfCorrectNumber == count){
            break;
        }
    }
    for (int i = 0; i < countOfCorrectNumber; i++){
        int element = QRandomGenerator::global()->bounded(lowerBound, upperBound+1);
        while(isContainElement(element, result) || element % divisor != 0){
            element = QRandomGenerator::global()->bounded(lowerBound, upperBound+1);
        }
        result.push_back(element);
    }

    return result;
}


void MySetsCalc::on_pushButton_addSet_clicked()
{
    QString setName;
    int countElement, downLimit, upperLimit;
    std::vector<int> set;

    if (!isDataCorrect(setName, countElement, downLimit, upperLimit, set)){
        return;
    }
    if (isForbiddenSymbol(setName)){
        ui->lineEdit_nameOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Название множества содержит запрещенные символы.";
        errorMessage(text);
        return;
    }

    if (isNameEngaged(setName, allSets))
    {
        ui->lineEdit_nameOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Множество с таким именем уже существует.";
        errorMessage(text);
        return;
    }
    else
    {
        int count = set.size();
        MySet* newSet = new MySet(setName, count, downLimit, upperLimit, set);
        allSets.push_back(newSet);

        ui->lineEdit_nameOfSet->clear();
        ui->lineEdit_elementsOfSet->clear();
        ui->lineEdit_countElement->setText("0");
        ui->label_allCountingElement->setText("0");

        ui->listWidget_allSets->addItem(setName);
        syncManager->syncLists();

        ui->label_sets->show();
        ui->listWidget_allSets->show();
    }
}


void MySetsCalc::errorMessage(QString& text)
{
    QMessageBox::warning(this, tr("Ошибка"), text);
}


void MySetsCalc::createContextMenu(QListWidget* listWidget)
{
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(listWidget, &QListWidget::customContextMenuRequested, this, [this, listWidget](const QPoint& pos) {
        QListWidgetItem* item = listWidget->itemAt(pos);
        if (item) {
            QMenu menu(listWidget);

            QAction* editAction = menu.addAction("Изменить");
            QAction* delAction = menu.addAction("Удалить");

            connect(editAction, &QAction::triggered, this, [this, item]() {
                for (auto &set : allSets){
                    if (set->getName() == item->text()){

                        ui->stackedWidget_buttons->setCurrentIndex(1);

                        ui->lineEdit_countElement->setText(QString::number(set->getCount()));
                        ui->lineEdit_downLimit->setText(QString::number(set->getDownLim()));
                        ui->lineEdit_upperLimit->setText(QString::number(set->getUpperLim()));
                        ui->lineEdit_nameOfSet->setText(set->getName());

                        ui->label_sets->hide();
                        ui->listWidget_allSets->hide();

                        std::vector<int> vectSet = set->getSet();
                        if (!vectSet.empty())
                        {
                            QString setElementInString = QString::number(vectSet[0]);
                            for (int element(1); element < int(vectSet.size()); element++){
                                setElementInString += ", " + QString::number(vectSet[element]);
                            }
                            ui->lineEdit_elementsOfSet->setText(setElementInString);
                        }
                        else
                            ui->lineEdit_elementsOfSet->setText("");
                        break;
                    }
                }
            });
            connect(delAction, &QAction::triggered, this, [this, item, listWidget]() {
                for (auto indexSet(0); indexSet < int(allSets.size()); indexSet++){
                    if (allSets[indexSet]->getName() == item->text()){
                        allSets[indexSet]->delSet();
                        allSets.erase(allSets.begin() + indexSet);
                        delete listWidget->takeItem(listWidget->row(item));
                        syncManager->syncLists();
                        break;
                    }
                }
                if (listWidget->count() == 0){
                    listWidget->hide();
                    ui->label_sets->hide();
                }
            });

            menu.exec(listWidget->mapToGlobal(pos));
        }
    });
}


bool MySetsCalc::isDataCorrect(QString &setName, int &countElement, int &downLimit, int &upperLimit, std::vector<int> &set)
{
    ui->lineEdit_countElement->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_downLimit->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_upperLimit->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_elementsOfSet->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_nameOfSet->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_divisor->setStyleSheet(defaultStyleSheet);

    bool isOk = true;

    setName = ui->lineEdit_nameOfSet->text();
    if(ui->lineEdit_nameOfSet->text().isEmpty())
    {
        ui->lineEdit_nameOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    bool isCountElementCorrect = true;
    countElement = ui->lineEdit_countElement->text().toInt(&isCountElementCorrect);
    if (!isCountElementCorrect || ui->lineEdit_countElement->text().isEmpty()){
        ui->lineEdit_countElement->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    bool isDownLimitCorrect = true;
    if (!ui->lineEdit_downLimit->text().isEmpty()){
        downLimit = ui->lineEdit_downLimit->text().toInt(&isDownLimitCorrect);
    } else {
         downLimit = ui->lineEdit_downLimit->placeholderText().toInt(&isDownLimitCorrect);
    }
    if (!isDownLimitCorrect){
        ui->lineEdit_downLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    bool isUpperLimitCorrect = true;
    if (!ui->lineEdit_upperLimit->text().isEmpty()){
        upperLimit = ui->lineEdit_upperLimit->text().toInt(&isUpperLimitCorrect);
    } else {
        upperLimit = ui->lineEdit_upperLimit->placeholderText().toInt(&isUpperLimitCorrect);
    }
    if (!isUpperLimitCorrect){
        ui->lineEdit_upperLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    QString setString = ui->lineEdit_elementsOfSet->text();
    if (!isStringSetToVector(setString, set)){
        ui->lineEdit_elementsOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }
    if (!isOk)
    {
        QString text = "Недостаточно данных для формирования множества или некорректный формат данных.";
        errorMessage(text);
        return false;
    }
    else if (int(set.size()) > countElement)
    {
        ui->lineEdit_elementsOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Количество элементов больше заданного.";
        errorMessage(text);
        return false;
    }
    else if (set.empty() && countElement > 0)
    {
        ui->lineEdit_elementsOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Недостаточно элементов множества.";
        errorMessage(text);
        return false;
    }
    else if (downLimit > upperLimit){
        ui->lineEdit_downLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        ui->lineEdit_upperLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Неверно задан диапазон.";
        errorMessage(text);
        return false;
    }
    else if (setName.contains(' ')){
        ui->lineEdit_nameOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Пожалуйста, не используйте пробелы в названии множества.";
        errorMessage(text);
        return false;
    }
    for (auto &element : set){
        if (element < downLimit || element > upperLimit){
            ui->lineEdit_elementsOfSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
            QString text = "Элементы не соответствуют заданному диапазону.";
            errorMessage(text);
            return false;
        }
    }
    return true;
}

void MySetsCalc::calculateFormul(QStringList formula)
{
    QString formul = "";
    for (auto &element : formula){
        formul += element;
    }

    int index = 0;
    QList<QString> tokenizedFormul = tokenize(formul);

    TreeNode* expansionTree = buildExpressionTree(tokenizedFormul, ui->listWidget_allSets, index);
    if (expansionTree == nullptr || !isExpansionCorrect(tokenizedFormul)){
        ui->lineEdit_formula->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Ошибка в формуле.";
        errorMessage(text);
        return;
    }
    if (!isUniverseCorrect(tokenizedFormul, allSets)){
        ui->lineEdit_formula->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "У множеств должен быть один универсум.";
        errorMessage(text);
        return;
    }

    MySet resultSet = evaluateExpressionTree(expansionTree, allSets);

    if (ui->comboBox_newOrEdit->currentIndex() == 0){
        for (int indexSet = 0; indexSet < int(allSets.size()); indexSet++){
            if (allSets[indexSet]->getName() == ui->lineEdit_nameOfNewSet->text()){
                return;
            }
        }
        QString setName = ui->lineEdit_nameOfNewSet->text();
        MySet* resultSetPtr = new MySet(setName, resultSet.getCount(),
                                        resultSet.getDownLim(), resultSet.getUpperLim(), resultSet.getSet());
        allSets.push_back(resultSetPtr);
        ui->listWidget_allSets->addItem(setName);
        syncManager->syncLists();
    }
    else{
        for (int indexSet = 0; indexSet < int(allSets.size()); indexSet++){
            if (allSets[indexSet]->getName() == ui->comboBox_chooseSetForCalc->currentText()){
                allSets[indexSet]->setCount(resultSet.getCount());
                allSets[indexSet]->setSet(resultSet.getSet());
            }
        }
    }
    ui->lineEdit_calcResult->setText(vectToString(resultSet.getSet()));
}


void MySetsCalc::on_lineEdit_elementsOfSet_textChanged(const QString &text)
{
    if (!text.isEmpty() && text.endsWith(' ') && !text.endsWith(", ") && !text.endsWith(','))
    {
        ui->lineEdit_elementsOfSet->setText(text.left(text.length()-1) + ", ");
    }
    std::vector<int> set;
    if (isStringSetToVector(text, set))
    {
        bool isCountElementCorrect = true;
        int count = ui->lineEdit_countElement->text().toInt(&isCountElementCorrect);
        ui->label_allCountingElement->setText(QString::number(set.size()));
        if (!isCountElementCorrect || !ui->lineEdit_countElement->text().isEmpty()){
            if (int(set.size()) >= count){
                ui->lineEdit_countElement->setText(QString::number(set.size()));
            }
        }
    }
}


void MySetsCalc::on_pushButton_applyChanges_clicked()
{
    QString setName;
    int countElement, downLimit, upperLimit;
    std::vector<int> set;

    if (!isDataCorrect(setName, countElement, downLimit, upperLimit, set)){
        return;
    }

    QMessageBox messageBox(QMessageBox::Question,
                                   tr("Вы уверены?"),
                                   tr("Точно изменить текущее множество?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
            messageBox.setButtonText(QMessageBox::No, tr("Нет"));

    if (messageBox.exec() == QMessageBox::Yes){
        for (auto &editedSet : allSets){
            if (editedSet->getName() == ui->listWidget_allSets->currentItem()->text()){
                ui->listWidget_allSets->currentItem()->setText(setName);
                editedSet->setName(setName);
                int count = set.size();
                editedSet->setCount(count);
                editedSet->setDownLim(downLimit);
                editedSet->setUpperLim(upperLimit);
                editedSet->setSet(set);
                break;
            }
        }
        ui->stackedWidget_buttons->setCurrentIndex(0);

        ui->lineEdit_countElement->setText("0");
        ui->lineEdit_downLimit->setText("");
        ui->lineEdit_upperLimit->setText("");
        ui->lineEdit_elementsOfSet->setText("");
        ui->lineEdit_nameOfSet->setText("");

        if (ui->listWidget_allSets->count() > 0){
            ui->label_sets->show();
            ui->listWidget_allSets->show();
        }
    }
}


void MySetsCalc::on_pushButton_cancelChanges_clicked()
{
    QMessageBox messageBox(QMessageBox::Question,
                                   tr("Вы уверены?"),
                                   tr("Внесенные изменения не сохранятся."),
                                   QMessageBox::Yes | QMessageBox::No,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
            messageBox.setButtonText(QMessageBox::No, tr("Нет"));

    if (messageBox.exec() == QMessageBox::Yes){
        ui->stackedWidget_buttons->setCurrentIndex(0);

        ui->lineEdit_countElement->setText("0");
        ui->lineEdit_elementsOfSet->setText("");
        ui->lineEdit_nameOfSet->setText("");

        if (ui->listWidget_allSets->count() > 0){
            ui->label_sets->show();
            ui->listWidget_allSets->show();
        }
    }
}


void MySetsCalc::on_pushButton_randomizeSet_clicked()
{
    ui->lineEdit_countElement->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_downLimit->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_upperLimit->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_elementsOfSet->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_nameOfSet->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_divisor->setStyleSheet(defaultStyleSheet);

    bool isOk = true;

    bool isCountElementCorrect = true;
    int countElement = ui->lineEdit_countElement->text().toInt(&isCountElementCorrect);
    if (!isCountElementCorrect || ui->lineEdit_countElement->text().isEmpty()){
        ui->lineEdit_countElement->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    bool isDownLimitCorrect = true;
    int downLimit;
    if (!ui->lineEdit_downLimit->text().isEmpty()){
        downLimit = ui->lineEdit_downLimit->text().toInt(&isDownLimitCorrect);
    } else {
         downLimit = ui->lineEdit_downLimit->placeholderText().toInt(&isDownLimitCorrect);
    }
    if (!isDownLimitCorrect){
        ui->lineEdit_downLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    bool isUpperLimitCorrect = true;
    int upperLimit;
    if (!ui->lineEdit_upperLimit->text().isEmpty()){
        upperLimit = ui->lineEdit_upperLimit->text().toInt(&isUpperLimitCorrect);
    } else {
        upperLimit = ui->lineEdit_upperLimit->placeholderText().toInt(&isUpperLimitCorrect);
    }
    if (!isUpperLimitCorrect){
        ui->lineEdit_upperLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        isOk = false;
    }

    int divisor = 1;
    if (ui->checkBox_divisor->isChecked()){
        bool isDivisorCorrect;
        divisor = ui->lineEdit_divisor->text().toInt(&isDivisorCorrect);
        if (!isDivisorCorrect || ui->lineEdit_divisor->text().isEmpty()){
            ui->lineEdit_divisor->setStyleSheet("QLineEdit { border: 1px solid red; }");
            isOk = false;
        }
    }

    if (!isOk)
    {
        QString text = "Недостаточно данных для формирования множества или некорректный формат данных.";
        errorMessage(text);
        return;
    }
    else if (downLimit > upperLimit){
        ui->lineEdit_downLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        ui->lineEdit_upperLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Неверно задан диапазон.";
        errorMessage(text);
        return;
    }
    else if (ui->checkBox_onlyPositiveNumber->isChecked() && upperLimit < 0)
    {
        ui->lineEdit_upperLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Некорректный диапазон для выбранных условий.";
        errorMessage(text);
        return;
    }
    else if (ui->checkBox_onlyNegativeNumber->isChecked() && downLimit > 0)
    {
        ui->lineEdit_downLimit->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Некорректный диапазон для выбранных условий.";
        errorMessage(text);
        return;
    }

    if (ui->checkBox_onlyPositiveNumber->isChecked() && downLimit < 0)
        downLimit = 1;
    else if (ui->checkBox_onlyNegativeNumber->isChecked() && upperLimit > 0)
        upperLimit = -1;

    std::vector<int> randomSets = generateRandomNumbers(downLimit, upperLimit, countElement, divisor);

    if (!randomSets.empty()){
        QString stringVectOfElements = QString::number(randomSets[0]);
        for (int element(1); element < int(randomSets.size()); element++){
            stringVectOfElements += ", " + QString::number(randomSets[element]);
        }
        ui->lineEdit_elementsOfSet->setText(stringVectOfElements);
    }
    else{
        ui->lineEdit_elementsOfSet->setText("");
    }
}


void MySetsCalc::on_checkBox_onlyPositiveNumber_stateChanged(int arg1)
{
    if (ui->checkBox_onlyNegativeNumber->isChecked() && arg1){
        ui->checkBox_onlyNegativeNumber->setChecked(false);
    }
}


void MySetsCalc::on_checkBox_onlyNegativeNumber_stateChanged(int arg1)
{
    if (ui->checkBox_onlyPositiveNumber->isChecked() && arg1){
        ui->checkBox_onlyPositiveNumber->setChecked(false);
    }
}


void MySetsCalc::on_pushButton_printAssociation_clicked()
{
    printStringSign(ui->lineEdit_formula, "∪");
}


void MySetsCalc::on_pushButton_printIntersection_clicked()
{
    printStringSign(ui->lineEdit_formula, "∩");
}


void MySetsCalc::on_pushButton_printDifference_clicked()
{
    printStringSign(ui->lineEdit_formula, "\\");
}


void MySetsCalc::on_pushButton_printSymmetricalDifference_clicked()
{
    printStringSign(ui->lineEdit_formula, "∆");
}


void MySetsCalc::on_pushButton_printAddition_clicked()
{
    ui->lineEdit_formula->setStyleSheet(styleSheet());

    QString currentText = ui->lineEdit_formula->text();
    int cursorCurrentPos = ui->lineEdit_formula->cursorPosition();
    int indexBkt = findStartAddition(currentText.left(cursorCurrentPos));
    if (indexBkt <= 0){
        currentText = "¬" + currentText.right(currentText.size() - indexBkt);
    }
    else if (indexBkt != -1){
        currentText = currentText.left(indexBkt + 1) + "¬" + currentText.right(currentText.size() - indexBkt - 1);
    }
    else{
        ui->lineEdit_formula->setStyleSheet("QLineEdit { border: 1px solid red; }");
        return;
    }
    ui->lineEdit_formula->setText(currentText);
}


void MySetsCalc::on_listWidget_setsToChoose_itemDoubleClicked(QListWidgetItem *item)
{
    printStringSign(ui->lineEdit_formula, item->text());
}


void MySetsCalc::on_pushButton_calculateFormul_clicked()
{
    ui->lineEdit_formula->setStyleSheet(defaultStyleSheet);

    ui->comboBox_chooseSetForCalc->setStyleSheet(defaultStyleSheetComboBox);
    ui->comboBox_chooseSetForCalc->setFont(defaultFont);

    ui->lineEdit_nameOfNewSet->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_nameOfNewSet->setFont(defaultFont);

    if (ui->lineEdit_formula->text().isEmpty()){
        ui->lineEdit_formula->setStyleSheet("QLineEdit { border: 1px solid red; }");
    }

    if (ui->comboBox_newOrEdit->currentIndex() == 1 && ui->comboBox_chooseSetForCalc->currentIndex() == 0){
        ui->comboBox_chooseSetForCalc->setStyleSheet("QComboBox { border: 1px solid red; }");
        QString text = "Нужно выбрать множество.";
        errorMessage(text);
        return;
    }
    else if (ui->comboBox_newOrEdit->currentIndex() == 0 && ui->lineEdit_nameOfNewSet->text().isEmpty()){
        ui->lineEdit_nameOfNewSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Введите название нового множества.";
        errorMessage(text);
        return;
    }
    else if (isForbiddenSymbol(ui->lineEdit_nameOfNewSet->text())){
        ui->lineEdit_nameOfNewSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Название множества содержит запрещенные символы.";
        errorMessage(text);
        return;
    }
    else if (isNameEngaged(ui->lineEdit_nameOfNewSet->text(), allSets)){
        ui->lineEdit_nameOfNewSet->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Множество с таким именем уже существует.";
        errorMessage(text);
        return;
    }

    QStringList formula = ui->lineEdit_formula->text().split(' ');
    if (!isBktCorrect(formula)){
        ui->lineEdit_formula->setStyleSheet("QLineEdit { border: 1px solid red; }");
        QString text = "Неправильно расставлены скобки.";
        errorMessage(text);
        return;
    }

    calculateFormul(formula);
}


void MySetsCalc::on_comboBox_newOrEdit_currentIndexChanged(int index)
{
    if (index == 0){
        ui->stackedWidget_newOrEdit->setCurrentIndex(0);
        ui->comboBox_chooseSetForCalc->hide();
    }
    else if (index == 1){
        ui->stackedWidget_newOrEdit->setCurrentIndex(1);
        ui->comboBox_chooseSetForCalc->show();
    }
    ui->comboBox_chooseSetForCalc->setStyleSheet(defaultStyleSheetComboBox);
    ui->comboBox_chooseSetForCalc->setFont(defaultFont);

    ui->lineEdit_nameOfNewSet->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_nameOfNewSet->setFont(defaultFont);
}


void MySetsCalc::on_comboBox_chooseSetForCalc_currentIndexChanged(const QString &setName)
{
    ui->lineEdit_calcResult->clear();
    if (setName == "Выбрать множество"){
        ui->lineEdit_oldSetElement->setText("");
        return;
    }
    else {
        for (int indexSet = 0; indexSet < int(allSets.size()); indexSet++){
            if (setName == allSets[indexSet]->getName()){
                QString text = vectToString(allSets[indexSet]->getSet());
                ui->lineEdit_oldSetElement->setText(text);
                break;
            }
        }
    }
}


void MySetsCalc::on_pushButton_clear_clicked()
{
    ui->lineEdit_formula->setStyleSheet(defaultStyleSheet);
    ui->lineEdit_formula->clear();
}


void MySetsCalc::on_pushButton_openBracket_clicked()
{
    printStringSign(ui->lineEdit_formula, "(");
}


void MySetsCalc::on_pushButton_closeBracket_clicked()
{
    printStringSign(ui->lineEdit_formula, ")");
}


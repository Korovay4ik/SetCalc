#ifndef MYSET_H
#define MYSET_H

#include <vector>
#include <QPair>
#include <QString>
#include <QDebug>

class MySet
{
public:
    MySet(QString name, int count, int downLim, int upperLim, std::vector<int> set);
    MySet() {};

    void setName(QString name){this->name = name;};
    QString getName(){return this->name;};

    void setCount(int count){this->count = count;};
    int getCount(){return this->count;};

    void setDownLim(int downLim){this->limit.first = downLim;};
    void setUpperLim(int upperLim){this->limit.second = upperLim;};
    int getDownLim(){return this->limit.first;};
    int getUpperLim(){return this->limit.second;};

    void setSet(std::vector<int> set){this->set = set;};
    std::vector<int> getSet() {return this->set;};

    void delSet();

    MySet& operator=(const MySet& otherSet);

    // Операция объединения
    void countAssociation(const MySet& otherSet);
    // Операция пересчения
    void countInersection(const MySet& otherSet);
    // Операция разности
    void countDifference(const MySet& otherSet);
    // Операция симметрической разности
    void countSymmetricalDifference(const MySet& otherSet);
    // Операция дополнения
    void countAddition();

private:
    std::vector<int> set;
    QString name = "";
    int count = 0;
    QPair<int, int> limit;

};

#endif // MYSET_H

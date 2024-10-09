#include "myset.h"

MySet::MySet(QString name, int count, int downLim, int upperLim, std::vector<int> set)
{
    this->name = name;
    this->count = count;
    this->setDownLim(downLim);
    this->setUpperLim(upperLim);
    this->set = set;
}

void MySet::delSet()
{
    set.clear();
}

MySet &MySet::operator=(const MySet &otherSet)
{
    if (this != &otherSet)
    {
        this->name = otherSet.name;
        this->count = otherSet.count;
        this->limit =  otherSet.limit;
        this->set = otherSet.set;
    }

    return *this;
}

void MySet::countAssociation(const MySet &otherSet)
{
    for (auto &element : otherSet.set){
        if (std::find(this->set.begin(), this->set.end(), element) == this->set.end()){
            this->set.push_back(element);
            this->count++;
        }
    }
}

void MySet::countInersection(const MySet &otherSet)
{
    std::vector<int> newSet;
    for (auto &element : otherSet.set){
        if (std::find(this->set.begin(), this->set.end(), element) != this->set.end()){
            newSet.push_back(element);
        }
    }
    this->set = newSet;
    this->count = newSet.size();
}

void MySet::countDifference(const MySet &otherSet)
{
    for (auto &element : otherSet.set){
        if (std::find(this->set.begin(), this->set.end(), element) != this->set.end()){
            this->set.erase(std::find(this->set.begin(), this->set.end(), element));
        }
    }
    this->count = this->set.size();
}

void MySet::countSymmetricalDifference(const MySet &otherSet)
{
    for (auto &element : otherSet.set){
        if (std::find(this->set.begin(), this->set.end(), element) == this->set.end()){
            this->set.push_back(element);
            this->count++;
        }
        else{
            this->set.erase(std::find(this->set.begin(), this->set.end(), element));
            this->count--;
        }
    }
}

void MySet::countAddition()
{
    std::vector<int> newSet;
    for (int element = this->limit.first; element < this->limit.second; element++){
        if (std::find(this->set.begin(), this->set.end(), element) == this->set.end()){
            newSet.push_back(element);
        }
    }
    this->set = newSet;
    this->count = newSet.size();
}

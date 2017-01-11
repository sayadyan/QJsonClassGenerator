#ifndef CLASSGENERATEINFO_H
#define CLASSGENERATEINFO_H

#include <QString>
#include <QList>
#include <QPair>

class ClassGenerateInfo
{
public:
    ClassGenerateInfo();
    ClassGenerateInfo(const QString& name);
    
    void addFiled(const QString& name, const QString& type);
    
    QString className() const;
    void setClassName(const QString &className);
    
    QList<QPair<QString, QString> > fields() const;
    
private:
    QString mClassName;
    QList<QPair<QString,QString>> mFields;
};

#endif // CLASSGENERATEINFO_H

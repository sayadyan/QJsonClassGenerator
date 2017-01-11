#include "classgenerateinfo.h"

ClassGenerateInfo::ClassGenerateInfo()
{
    
}

ClassGenerateInfo::ClassGenerateInfo(const QString &name)
    : mClassName(name)
{
}

void ClassGenerateInfo::addFiled(const QString &name, const QString &type)
{
    mFields.push_back(QPair<QString,QString>(name, type));
}

QString ClassGenerateInfo::className() const
{
    return mClassName;
}

void ClassGenerateInfo::setClassName(const QString &className)
{
    mClassName = className;
}

QList<QPair<QString, QString> > ClassGenerateInfo::fields() const
{
    return mFields;
}

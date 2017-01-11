#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "classgenerateinfo.h"

class Generator : public QObject
{
    Q_OBJECT
public:
    explicit Generator(QObject *parent = 0);
    
    void build(const QString& classesNamePrefix,
               const QString& rootClassName,
               const QString& json,
               const QMap<QString,QString>& jmap,
               bool isExport);
    
signals:
    void error(QString info);
    void success(const QByteArray& result);
    
private:
    void generateAllFiles();
    void generateFile(const ClassGenerateInfo& cls);
    
    void generateClass(const QString& classesNamePrefix,
                       const QString& name,
                       const QJsonObject& obj,
                       const QMap<QString,QString>& jmap);
    void generateFakeArrayClass(const QString& classesNamePrefix,
                                const QString& name,
                                const QJsonArray& arr,
                                const QMap<QString,QString>& jmap);
    QString generateArrayType(const QString& classesNamePrefix,
                              const QString& name,
                              const QJsonArray& arr,
                              const QMap<QString,QString>& jmap);
    QString generateMapType(const QString& classesNamePrefix,
                              const QString& name,
                              const QJsonValue& val,
                              const QMap<QString,QString>& jmap);
    
    void appendIncludeToHeader(QByteArray& arr, const ClassGenerateInfo& cls);
    void appendJsonConstructorsAndInit(QByteArray& arr, const ClassGenerateInfo& cls);
    void appendFieldSetup(QByteArray& arr, const QString& field, const QString& type);
    QString mapFieldSetupGenerator(QByteArray& arr,
                                   const QString& type,
                                   QString indx,
                                   QString source,
                                   int spaceCount);
    QString listFieldSetupGenerator(QByteArray& arr,
                                    const QString& type,
                                    QString indx,
                                    QString source,
                                    int spaceCount);
    void appendGetterSetterPair(QByteArray& arr,
                                const QString& clsName,
                                const QString& field,
                                const QString& type);
    
    void exportToFiles();
    
    QString makeFirstLetterUpper(QString str);
    
private:
    QMap<QString,QByteArray> filesData;
    QList<ClassGenerateInfo> classInfos;
};

#endif // GENERATOR_H

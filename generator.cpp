#include "generator.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#include "inflector.h"

Generator::Generator(QObject *parent) : QObject(parent)
{
    
}

void Generator::build(const QString &classesNamePrefix,
                      const QString& rootClassName,
                      const QString& json,
                      const QMap<QString, QString> &jmap,
                      bool isExport)
{
    filesData.clear();
    classInfos.clear();
    
    QJsonDocument doc = QJsonDocument::fromJson(json.toLatin1());
    if (doc.isEmpty() || doc.isNull()) {
        emit error("Document is empty.");
        return;
    }
    
    if (doc.isArray()) {
        generateFakeArrayClass(classesNamePrefix, rootClassName, doc.array(), jmap);
    } else {
        generateClass(classesNamePrefix, rootClassName, doc.object(), jmap);
    }
    
    generateAllFiles();
    
    QByteArray result;
    for (const QString &fileName : filesData.keys()) {
        result.append(fileName).append("\n-------------------\n");
        result.append(filesData.value(fileName));
        result.append("\n");
    }
    
    if (isExport) {
        exportToFiles();
    }
    
    emit success(result);
}

void Generator::generateAllFiles()
{
    for (ClassGenerateInfo &c : classInfos) {
        generateFile(c);
    }
}

void Generator::generateFile(const ClassGenerateInfo &cls)
{
    QByteArray header;
    
    header.append("#ifndef ").append(cls.className().toUpper()).append("_H\n");
    header.append("#define ").append(cls.className().toUpper()).append("_H\n\n");
    
    appendIncludeToHeader(header, cls);
    header.append("\n");
    
    header.append("class ").append(cls.className()).append(" {\n");
    header.append("public:\n");
    // Add empty default constructor
    header.append("    ").append(cls.className()).append("() {}\n");
    // Add constructor from QJsonDocument
    header.append("    ").append(cls.className())
            .append("(const QJsonDocument& doc);\n");
    if (cls.fields().count() == 1 &&
            cls.fields().at(0).first == "list") {
        // Add constructor from QJsonArray
        header.append("    ").append(cls.className())
                .append("(const QJsonArray &arr);\n");
    }
    // Add constructor from QJsonObject
    header.append("    ").append(cls.className())
            .append("(const QJsonObject &obj);\n\n");
    if (cls.fields().count() == 1 &&
            cls.fields().at(0).first == "list") {
        // Add init from QJsonArray
        header.append("    ").append("void init")
                .append("(const QJsonArray &arr);\n");
    }
    // Add init from QJsonObject
    header.append("    ").append("void init")
            .append("(const QJsonObject &obj);\n\n");
    // Add getter/setter
    for (QPair<QString, QString> &p : cls.fields()) {
        // getter
        if (p.second.startsWith("QList") ||
                p.second.startsWith("QMap")) {
            // getter
            header.append("    ").append(p.second).append(" &").append(p.first)
                    .append("();\n");
            // setter
            header.append("    void set").append(makeFirstLetterUpper(p.first))
                    .append("(const ").append(p.second).append(" &")
                    .append(p.first).append(");\n\n");
        } else {
            header.append("    ").append(p.second).append(" ").append(p.first)
                    .append("() const;\n");
            // setter
            if (p.second.startsWith("QString")) {
                header.append("    void set").append(makeFirstLetterUpper(p.first))
                        .append("(const ").append(p.second).append(" &")
                        .append(p.first).append(");\n\n");
            } else {
                header.append("    void set").append(makeFirstLetterUpper(p.first))
                        .append("(").append(p.second).append(" ")
                        .append(p.first).append(");\n\n");
            }
        }
    }
    
    // Add fields
    header.append("private:\n");
    for (QPair<QString, QString> &p : cls.fields()) {
        header.append("    ").append(p.second).append(" m")
                .append(makeFirstLetterUpper(p.first)).append(";\n");
    }
    
    header.append("};\n\n");
    
    header.append("#endif // ").append(cls.className().toUpper()).append("_H\n");
    
    filesData.insert(cls.className().toLower()+".h",
                     header);
    
    QByteArray cpp;
    
    cpp.append("#include \"" + cls.className().toLower()+".h\"\n\n");
    cpp.append("#include <QStringList>\n\n");
    
    appendJsonConstructorsAndInit(cpp, cls);
    
    for (QPair<QString, QString> &p : cls.fields()) {
        appendGetterSetterPair(cpp,
                               cls.className(),
                               p.first,
                               p.second);
    }
    
    cpp.append("\n");
    filesData.insert(cls.className().toLower()+".cpp",
                     cpp);
}

void Generator::generateClass(const QString &classesNamePrefix,
                              const QString &name,
                              const QJsonObject &obj,
                              const QMap<QString, QString> &jmap)
{
    ClassGenerateInfo info(name);
    
    for (QString &key : obj.keys()) {
        if (jmap.contains(key)) {
            info.addFiled("pairs", "QMap<QString,"+
                          generateMapType(classesNamePrefix,
                                          classesNamePrefix + jmap.value(key),
                                          obj.value(key), jmap)
                          +">");
            break;
        } else if (obj.value(key).isDouble()) {
            if (!QString::number(obj.value(key).toDouble(), 'f').contains(QRegExp("(\\.[0]+$)|(^[0-9]+$)"))) {
                info.addFiled(key, "double");
            } else {
                // JSON int 64-bit
                info.addFiled(key, "qint64");
            }
        } else if (obj.value(key).isString()) {
            bool okI, okD;
            obj.value(key).toString().toLongLong(&okI);
            obj.value(key).toString().toDouble(&okD);
            if (okI) {
                info.addFiled(key, "qint64");
            } else if (okD) {
                info.addFiled(key, "double");
            } else {
                info.addFiled(key, "QString");
            }
        } else if (obj.value(key).isNull()) {
            info.addFiled(key, "void *");
        } else if (obj.value(key).isBool()) {
            info.addFiled(key, "bool");
        } else if (obj.value(key).isArray()) {
            QString ifObjThenName = Inflector::singularize(key);
            ifObjThenName = classesNamePrefix + ifObjThenName.replace(0, 1, ifObjThenName.left(1).toUpper());
            QString arrayType = generateArrayType(classesNamePrefix, ifObjThenName, obj.value(key).toArray(), jmap);
            info.addFiled(key, "QList<" + arrayType + ">");
        } else if (obj.value(key).isObject()) {
            QString newClassName = classesNamePrefix + key.replace(0, 1, key.left(1).toUpper());
            generateClass(classesNamePrefix,
                          newClassName,
                          obj.value(key).toObject(),
                          jmap);
            info.addFiled(key, newClassName);
        }
    }
    
    classInfos.push_front(info);
}

void Generator::generateFakeArrayClass(const QString &classesNamePrefix,
                                       const QString &name,
                                       const QJsonArray &arr,
                                       const QMap<QString, QString> &jmap)
{
    ClassGenerateInfo info(name + "Array");
    
    QString arrayType = generateArrayType(classesNamePrefix, name, arr, jmap);
    info.addFiled("list", "QList<" + arrayType + ">");
    classInfos.push_front(info);
}

QString Generator::generateArrayType(const QString& classesNamePrefix,
                                     const QString &name,
                                     const QJsonArray &arr,
                                     const QMap<QString, QString> &jmap)
{
    if (arr.isEmpty()) {
        return "void *";
    }
    
    QJsonValue val =  arr.at(0);
    if (val.isBool()) {
        return "bool";
    } else if (val.isDouble()) {
        if (!QString::number(val.toDouble(), 'f').contains(QRegExp("(\\.[0]+$)|(^[0-9]+$)"))) {
            return "double";
        } else {
            // JSON int 64-bit
            return "qint64";
        }
    } else if (val.isString()) {
        bool okI, okD;
        val.toString().toLongLong(&okI);
        val.toString().toDouble(&okD);
        if (okI) {
            return "qint64";
        } else if (okD) {
            return "double";
        } else {
            return "QString";
        }
    } else if (val.isNull()) {
        return "void *";
    } else if (val.isObject()) {
        generateClass(classesNamePrefix,
                      name,
                      val.toObject(),
                      jmap);
        return name;
    } else if (val.isArray()) {
        return "QList<" + generateArrayType(classesNamePrefix,
                                            name + "Sub",
                                            val.toArray(),
                                            jmap) + ">";
    }
    
    return "void *";
}

QString Generator::generateMapType(const QString &classesNamePrefix,
                                   const QString &name,
                                   const QJsonValue &val,
                                   const QMap<QString, QString> &jmap)
{   
    if (val.isBool()) {
        return "bool";
    } else if (val.isDouble()) {
        if (!QString::number(val.toDouble(), 'f').contains(QRegExp("(\\.[0]+$)|(^[0-9]+$)"))) {
            return "double";
        } else {
            // JSON int 64-bit
            return "qint64";
        }
    } else if (val.isString()) {
        bool okI, okD;
        val.toString().toLongLong(&okI);
        val.toString().toDouble(&okD);
        if (okI) {
            return "qint64";
        } else if (okD) {
            return "double";
        } else {
            return "QString";
        }
    } else if (val.isNull()) {
        return "void *";
    } else if (val.isObject()) {
        generateClass(classesNamePrefix,
                      name,
                      val.toObject(),
                      jmap);
        return name;
    } else if (val.isArray()) {
        return "QList<" + generateArrayType(classesNamePrefix,
                                            name + "Sub",
                                            val.toArray(),
                                            jmap) + ">";
    }
    
    return "void *";
}

void Generator::appendIncludeToHeader(QByteArray &arr, const ClassGenerateInfo &cls)
{
    static QStringList defaultTypes = {"bool", "double", "qint64", "void *"};
    static QStringList qtDefaultTypes = {"QString"};
    static QStringList qtTemplateDefaultTypes = {"QList", "QMap"};
    
    QStringList included = {};
    
    for (QPair<QString, QString> &p : cls.fields()) {
        if (defaultTypes.contains(p.second)) {
            continue;
        }
        if (qtDefaultTypes.contains(p.second)) {
            if (!included.contains(p.second)) {
                arr.append("#include <").append(p.second).append(">\n");
                included.append(p.second);
            }
            continue;
        }
        if (qtTemplateDefaultTypes.contains(p.second.split("<")[0])) {
            QStringList splitResult = p.second.split(QRegExp("[<,]"));
            for (QString &s : splitResult) {
                s.replace("<","").replace(">","");
                if (!included.contains(s)) {
                    if (qtDefaultTypes.contains(s) ||
                            qtTemplateDefaultTypes.contains(s)) {
                        arr.append("#include <").append(s).append(">\n");
                    } else {
                        if (!defaultTypes.contains(s)) {
                            arr.append("#include \"").append(s.toLower()).append(".h\"\n");
                        }
                    }
                    included.append(s);
                }
            }
            continue;
        }
        // own type
        if (!included.contains(p.second.toLower())) {
            arr.append("#include \"").append(p.second.toLower()).append(".h\"\n");
            included.append(p.second.toLower());
        }
    }
    
    arr.append("#include <QJsonDocument>\n");
    arr.append("#include <QJsonObject>\n");
    arr.append("#include <QJsonArray>\n");
}

void Generator::appendJsonConstructorsAndInit(QByteArray &arr,
                                              const ClassGenerateInfo &cls)
{
    // Constructor QJsonDocument
    arr.append(cls.className()).append("::").append(cls.className())
            .append("(const QJsonDocument &doc)\n");
    arr.append("{\n");
    if (cls.fields().count() == 1 &&
            cls.fields().at(0).first == "list") {
        arr.append("    if (doc.isArray()) {\n");
        arr.append("        init(doc.array());\n");
        arr.append("    } else {\n");
        arr.append("        init(doc.object());\n");
        arr.append("    }\n");
    } else {
        arr.append("    init(doc.object());\n");
    }
    arr.append("}\n\n");
    if (cls.fields().count() == 1 &&
            cls.fields().at(0).first == "list") {
        // Constructor QJsonArray
        arr.append(cls.className()).append("::").append(cls.className())
                .append("(const QJsonArray &arr)\n");
        arr.append("{\n");
        arr.append("    init(arr);\n");
        arr.append("}\n\n");
    }
    // Constructor QJsonObject
    arr.append(cls.className()).append("::").append(cls.className())
            .append("(const QJsonObject &obj)\n");
    arr.append("{\n");
    arr.append("    init(obj);\n");
    arr.append("}\n\n");
    // Init QJsonArray
    if (cls.fields().count() == 1 &&
            cls.fields().at(0).first == "list") {
        arr.append("void ").append(cls.className())
                .append("::init(const QJsonArray &arr)\n");
        arr.append("{\n");
        arr.append("    for (const QJsonValue &val : arr) {\n");
        QString subType = cls.fields().at(0).second.mid(6);
        subType = subType.mid(0, subType.count()-1);
        arr.append("        mList.push_back(")
                .append(subType)
                .append("(val.toObject()));\n");
        arr.append("    }\n");
        arr.append("}\n\n");
    } 
    // Init QJsonObject
    arr.append("void ").append(cls.className())
            .append("::init(const QJsonObject &obj)\n");
    arr.append("{\n");

    if (cls.fields().count() == 0) {
        // do nothing
    } else if (cls.fields().count() == 1 &&
            cls.fields().at(0).first == "pairs") {
        // constructor for QMap
        appendFieldSetup(arr, cls.fields().at(0).first,
                         cls.fields().at(0).second);
    } else {
        arr.append("    for (QString &key : obj.keys()) {\n");
        
        bool first = true;
        for (QPair<QString, QString> &p : cls.fields()) {
            if (first) {
                arr.append("        if (key == \"").append(p.first).append("\") {\n");
                first = false;
            } else {
                arr.append("        } else if (key == \"").append(p.first)
                        .append("\") {\n");
            }
            appendFieldSetup(arr, p.first, p.second);
        }
        
        arr.append("        }\n");
        arr.append("    }\n");
    }
    
    arr.append("}\n\n");
}

void Generator::appendFieldSetup(QByteArray &arr, const QString &field, const QString &type)
{
    if (type == "void *") {
        arr.append("            set").append(makeFirstLetterUpper(field))
                .append("(nullptr);\n");
    } else if (type == "double") {
        arr.append("            if (obj.value(key).isString()) {\n");
        arr.append("                set").append(makeFirstLetterUpper(field))
                .append("(obj.value(key).toString().toDouble());\n");
        arr.append("            } else {\n");
        arr.append("                set").append(makeFirstLetterUpper(field))
                .append("(obj.value(key).toDouble());\n");
        arr.append("            }\n");
    } else if (type == "qint64") {
        arr.append("            if (obj.value(key).isString()) {\n");
        arr.append("                set").append(makeFirstLetterUpper(field))
                .append("((qint64)obj.value(key).toString().toDouble());\n");
        arr.append("            } else {\n");
        arr.append("                set").append(makeFirstLetterUpper(field))
                .append("((qint64)obj.value(key).toDouble());\n");
        arr.append("            }\n");
    } else if (type == "bool") {
        arr.append("            set").append(makeFirstLetterUpper(field))
                .append("(obj.value(key).toBool());\n");
    } else if (type == "QString") {
        arr.append("            set").append(makeFirstLetterUpper(field))
                .append("(obj.value(key).toString());\n");
    } else if (type.left(5) == "QList") {
        listFieldSetupGenerator(arr, type, "0", "obj.value(key).toArray()", 12);
        arr.append("            set").append(makeFirstLetterUpper(field))
                .append("(par0);\n");
    } else if (type.left(4) == "QMap") {
        mapFieldSetupGenerator(arr, type, "0", "obj", 4);
        arr.append("    set").append(makeFirstLetterUpper(field))
                .append("(par0);\n");
    } else {
        // custom type
        arr.append("            set").append(makeFirstLetterUpper(field))
                .append("(").append(type)
                .append("(obj.value(key).toObject()));\n");
    }
}

QString Generator::mapFieldSetupGenerator(QByteArray &arr,
                                          const QString &type,
                                          QString indx,
                                          QString source,
                                          int spaceCount)
{
    arr.append(spaceCount, ' ').append("const QJsonObject obj")
            .append(indx) .append(" = ").append(source).append(";\n");
    arr.append(spaceCount, ' ').append(type).append(" par")
            .append(indx).append(";\n");
    arr.append(spaceCount, ' ').append("for (QString &key").append(indx)
            .append(" : obj").append(indx).append(".keys()) {\n");
    
    QString appendData;
    QString newType = type.mid(13);
    newType = newType.mid(0, newType.count() - 1);
    
    if (newType == "void *") {
        appendData = "nullptr";
    } else if (newType == "double") {
        appendData = "(obj"+indx+".value(key"+indx+").isString() ? obj"+
                indx+".value(key"+indx+").toString().toDouble() : obj"+
                indx+".value(key"+indx+").toDouble())";
    } else if (newType == "qint64") {
        appendData = "(qint64)(obj"+indx+".value(key"+indx+").isString() ? obj"+
                indx+".value(key"+indx+").toString().toDouble() : obj"+
                indx+".value(key"+indx+").toDouble())";
    } else if (newType == "bool") {
        appendData = "obj"+indx+".value(key"+indx+").toBool()";
    } else if (newType == "QString") {
        appendData = "obj"+indx+".value(key"+indx+").toString()";
    } else if (newType.left(5) == "QList") {
        appendData = listFieldSetupGenerator(arr, newType,
                                             QString::number(indx.toInt()+1),
                                             "obj"+indx+".value(key"+indx+").toArray()",
                                             spaceCount+4);
    } else if (newType.left(4) == "QMap") {
        // unreachable
        appendData = mapFieldSetupGenerator(arr, newType,
                                            QString::number(indx.toInt()+1),
                                            "obj"+indx+".value(key"+indx+").toObject()",
                                            spaceCount+4);
    } else {
        appendData = newType + "(obj"+indx+".value(key"+indx+").toObject())";
    }
    
    arr.append(spaceCount+4, ' ').append("par").append(indx)
            .append(".insert(key").append(indx).append(", ")
            .append(appendData).append(");\n");
    arr.append(spaceCount, ' ').append("}\n");
    
    return "par" + indx;
}

QString Generator::listFieldSetupGenerator(QByteArray &arr,
                                           const QString &type,
                                           QString indx,
                                           QString source,
                                           int spaceCount)
{
    arr.append(spaceCount, ' ').append("const QJsonArray arr")
            .append(indx).append(" = ").append(source).append(";\n");
    arr.append(spaceCount, ' ').append(type).append(" par")
            .append(indx).append(";\n");
    arr.append(spaceCount, ' ').append("for (const QJsonValue &val")
            .append(indx).append(" : arr").append(indx)
            .append(") {\n");
    
    QString appendData;
    QString newType = type.mid(6);
    newType = newType.mid(0, newType.count()-1);
    if (newType == "void *") {
        appendData = "nullptr";
    } else if (newType == "double") {
        appendData = "(val"+indx+".isString() ? val"+indx+
                ".toString().toDouble() : val"+indx+".toDouble())";
    } else if (newType == "qint64") {
        appendData = "(qint64)(val"+indx+".isString() ? val"+indx+
                ".toString().toDouble() : val"+indx+".toDouble())";
    } else if (newType == "bool") {
        appendData = "val"+indx+".toBool()";
    } else if (newType == "QString") {
        appendData = "val"+indx+".toString()";
    } else if (newType.left(5) == "QList") {
        appendData = listFieldSetupGenerator(arr, newType,
                                             QString::number(indx.toInt()+1),
                                             "val"+indx+".toArray()",
                                             spaceCount+4);
    } else if (newType.left(4) == "QMap") {
        // unreachable
        appendData = mapFieldSetupGenerator(arr, newType,
                                            QString::number(indx.toInt()+1),
                                            "val"+indx+".toObject()",
                                            spaceCount+4);
    } else {
        appendData = newType + "(val"+ indx + ".toObject())";
    }
    
    arr.append(spaceCount+4, ' ').append("par").append(indx)
            .append(".push_back(").append(appendData).append(");\n");
    
    arr.append(spaceCount, ' ').append("}\n");
    
    return "par" + indx;
}

void Generator::appendGetterSetterPair(QByteArray &arr,
                                       const QString &clsName,
                                       const QString &field,
                                       const QString &type)
{
    if (type.startsWith("QList") || type.startsWith("QMap")) {
        //getter
        arr.append(type).append(" &").append(clsName).append("::")
                .append(field).append("()\n");
        arr.append("{\n");
        arr.append("    return m").append(makeFirstLetterUpper(field))
                .append(";\n");
        arr.append("}\n\n");
        //setter
        arr.append("void ").append(clsName).append("::")
                .append("set").append(makeFirstLetterUpper(field))
                .append("(const ").append(type).append(" &")
                .append(field).append(")\n");
        arr.append("{\n");
        arr.append("    m").append(makeFirstLetterUpper(field))
                .append(" = ").append(field).append(";\n");
        arr.append("}\n\n");
    } else {
        //getter
        arr.append(type).append(" ").append(clsName).append("::")
                .append(field).append("() const\n");
        arr.append("{\n");
        arr.append("    return m").append(makeFirstLetterUpper(field))
                .append(";\n");
        arr.append("}\n\n");
        //setter
        if (type == "QString") {
            arr.append("void ").append(clsName).append("::")
                    .append("set").append(makeFirstLetterUpper(field))
                    .append("(const ").append(type).append(" &")
                    .append(field).append(")\n");
        } else {
            arr.append("void ").append(clsName).append("::")
                    .append("set").append(makeFirstLetterUpper(field))
                    .append("(").append(type).append(" ")
                    .append(field).append(")\n");
        }
        arr.append("{\n");
        arr.append("    m").append(makeFirstLetterUpper(field))
                .append(" = ").append(field).append(";\n");
        arr.append("}\n\n");
    }
}

void Generator::exportToFiles()
{
    QString dirName = QCoreApplication::applicationDirPath() + "/model";
    QDir dir(dirName);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    dir.mkdir(dirName);
    
    for (QString &key : filesData.keys()) {
        QFile f(dirName + "/" + key);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qDebug() << "Can't open file :( " << f.errorString();
            continue;
        }
        f.write(filesData.value(key));
        f.flush();
        f.close();
    }
}

QString Generator::makeFirstLetterUpper(QString str)
{
    return str.replace(0, 1, str.left(1).toUpper());
}

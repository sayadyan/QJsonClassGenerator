#ifndef INFLECTOR_H
#define INFLECTOR_H

#include <QString>
#include <QList>

#include "replacementrule.h"

class Inflector
{
public:
    static QString pluralize(const QString& word);
    static QString singularize(const QString& word);
    
private:
    Inflector();
    static void irregular(const QString& singular,
                   const QString& plural);
    
private:
    class Initializer
    {
    public:
        Initializer();
    };

    friend class Initializer;
    
private:
    static QList<ReplacementRule> plurals;
    static QList<ReplacementRule> singulars;
    static QList<QString> uncountables;
    
    static Initializer initializer;
};

#endif // INFLECTOR_H

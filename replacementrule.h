#ifndef REPLACEMENTRULE_H
#define REPLACEMENTRULE_H

#include <QString>
#include <QRegExp>

class ReplacementRule {
public:
    ReplacementRule(const QString& regexp, const QString& replacement) {
        if (regexp.contains("(?i)")) {
            this->regexp = QRegExp(QString(regexp).replace("(?i)", ""));
        } else {
            this->regexp = QRegExp(regexp);
        }
        this->regexp.setMinimal(false);
        this->r = replacement;
    }

    bool find(QString word) {
        return word.contains(regexp);
    }

    QString replace(QString word) {
        return word.replace(regexp, r);
    }
private:
    QRegExp regexp;
    QString r;
};
#endif // REPLACEMENTRULE_H

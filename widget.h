#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets>

#include "generator.h"

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    Widget(QWidget *parent = 0);
    ~Widget();
    
private slots:
    void generateClicked();
    void addMapItemClicked();
    void showContextMenu(const QPoint &pos);
    void deleteItem();
    
    void onError(QString info);
    void onSuccess(const QByteArray& result);
    
private:
    QTextEdit* mJsonEdit;
    QTextEdit* mClassEdit;
    
    QListWidget* mListWidget;
    QLineEdit* mRootClassName;
    QLineEdit* mClassesNamePrefix;
    QLineEdit* mLine1;
    QLineEdit* mLine2;
    
    Generator mGenerator;
};

#endif // WIDGET_H

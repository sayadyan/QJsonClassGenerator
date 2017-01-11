#include "widget.h"

#include "QtWidgets"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout();
    QHBoxLayout* hLayout = new QHBoxLayout();
    
    mJsonEdit = new QTextEdit();
    mJsonEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    mJsonEdit->setAcceptRichText(false);
    mJsonEdit->setPlaceholderText("Put json here...");
    vLayout->addWidget(mJsonEdit);
    
    mClassesNamePrefix = new QLineEdit();
    mClassesNamePrefix->setText("QM");
    hLayout->addWidget(new QLabel("Classes name prefix:"));
    hLayout->addWidget(mClassesNamePrefix);
    vLayout->addLayout(hLayout);
    
    hLayout = new QHBoxLayout();
    mRootClassName = new QLineEdit();
    mRootClassName->setText("QMRoot");
    hLayout->addWidget(new QLabel("Root class name:"));
    hLayout->addWidget(mRootClassName);
    vLayout->addLayout(hLayout);
    
    hLayout = new QHBoxLayout();
    hLayout->addWidget(new QLabel("Maps:"));
    mLine1 = new QLineEdit();
    mLine1->setPlaceholderText("First map element");
    hLayout->addWidget(mLine1);
    
    hLayout->addWidget(new QLabel(":"));
    
    mLine2 = new QLineEdit();
    mLine2->setPlaceholderText("Class name or _");
    hLayout->addWidget(mLine2);
    
    QPushButton* button = new QPushButton("Add");
    hLayout->addWidget(button);
    vLayout->addLayout(hLayout);
    QObject::connect(button,
                     &QPushButton::clicked,
                     this,
                     &Widget::addMapItemClicked);
    
    mListWidget = new QListWidget();
    vLayout->addWidget(mListWidget);
    mListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(mListWidget,
                     &QListWidget::customContextMenuRequested,
                     this,
                     &Widget::showContextMenu); 
    
    mExport = new QCheckBox("Export files");
    mExport->setChecked(false);
    vLayout->addWidget(mExport);
    
    button = new QPushButton("Generate");
    vLayout->addWidget(button);
    QObject::connect(button,
                     &QPushButton::clicked,
                     this,
                     &Widget::generateClicked);
    
    hLayout = new QHBoxLayout();
    hLayout->addLayout(vLayout);
    
    mClassEdit = new QTextEdit();
    mClassEdit->setPlaceholderText("Result from files");
    mClassEdit->setReadOnly(true);
    hLayout->addWidget(mClassEdit);
    
    QObject::connect(&mGenerator,
                     &Generator::error,
                     this,
                     &Widget::onError);
    QObject::connect(&mGenerator,
                     &Generator::success,
                     this,
                     &Widget::onSuccess);
    
    hLayout->setStretch(0,3);
    hLayout->setStretch(1,5);
    this->setLayout(hLayout);
}

Widget::~Widget()
{
    
}

void Widget::generateClicked()
{
    if (mRootClassName->text().isEmpty()) {
        return;
    }
    
    QMap<QString,QString> jmap;
    for (int i = 0; i < mListWidget->count(); ++i) {
        mListWidget->item(i);
        QListWidgetItem *item = mListWidget->item(i);
        QStringList list = item->text().split(":");
        jmap.insert(list.at(0), list.at(1));
    }
    
    mGenerator.build(mClassesNamePrefix->text(),
                     mRootClassName->text(),
                     mJsonEdit->toPlainText(),
                     jmap,
                     mExport->isChecked());
}

void Widget::addMapItemClicked()
{
    if (mLine1->text().isEmpty() ||
            mLine2->text().isEmpty()) {
        return;
    }
    
    mListWidget->addItem(mLine1->text() + ":" + mLine2->text());
}

void Widget::showContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = mListWidget->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    myMenu.addAction("Delete",  this, SLOT(deleteItem()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void Widget::deleteItem()
{
    // If multiple selection is on, we need to erase all selected items
    for (int i = 0; i < mListWidget->selectedItems().size(); ++i) {
        // Get curent item on selected row
        QListWidgetItem *item = mListWidget->takeItem(mListWidget->currentRow());
        // And remove it
        delete item;
    }
}

void Widget::onError(QString info)
{
    mClassEdit->setText(info);
}

void Widget::onSuccess(const QByteArray &result)
{
    mClassEdit->setText(result);
}

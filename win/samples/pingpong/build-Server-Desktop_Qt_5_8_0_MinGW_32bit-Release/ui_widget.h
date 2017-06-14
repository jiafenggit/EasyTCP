/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QTextEdit *txtedtMsg;
    QLabel *label_2;
    QLineEdit *ledtPort;
    QPushButton *btnOpen;
    QPushButton *btnClose;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(600, 400);
        txtedtMsg = new QTextEdit(Widget);
        txtedtMsg->setObjectName(QStringLiteral("txtedtMsg"));
        txtedtMsg->setGeometry(QRect(20, 10, 401, 371));
        txtedtMsg->setReadOnly(true);
        label_2 = new QLabel(Widget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(430, 10, 31, 16));
        ledtPort = new QLineEdit(Widget);
        ledtPort->setObjectName(QStringLiteral("ledtPort"));
        ledtPort->setGeometry(QRect(470, 10, 113, 20));
        btnOpen = new QPushButton(Widget);
        btnOpen->setObjectName(QStringLiteral("btnOpen"));
        btnOpen->setGeometry(QRect(480, 40, 75, 23));
        btnClose = new QPushButton(Widget);
        btnClose->setObjectName(QStringLiteral("btnClose"));
        btnClose->setGeometry(QRect(480, 70, 75, 23));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "EasyTcp - Server", Q_NULLPTR));
        label_2->setText(QApplication::translate("Widget", "\347\253\257\345\217\243", Q_NULLPTR));
        ledtPort->setText(QApplication::translate("Widget", "6948", Q_NULLPTR));
        btnOpen->setText(QApplication::translate("Widget", "\346\211\223\345\274\200", Q_NULLPTR));
        btnClose->setText(QApplication::translate("Widget", "\345\205\263\351\227\255", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H

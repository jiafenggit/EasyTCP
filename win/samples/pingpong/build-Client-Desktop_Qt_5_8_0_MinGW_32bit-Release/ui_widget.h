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
    QTextEdit *txtedtInput;
    QLineEdit *ledtHost;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *ledtPort;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect;
    QPushButton *btnSend;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(600, 400);
        txtedtMsg = new QTextEdit(Widget);
        txtedtMsg->setObjectName(QStringLiteral("txtedtMsg"));
        txtedtMsg->setGeometry(QRect(20, 10, 401, 251));
        txtedtMsg->setReadOnly(true);
        txtedtInput = new QTextEdit(Widget);
        txtedtInput->setObjectName(QStringLiteral("txtedtInput"));
        txtedtInput->setGeometry(QRect(20, 270, 401, 111));
        ledtHost = new QLineEdit(Widget);
        ledtHost->setObjectName(QStringLiteral("ledtHost"));
        ledtHost->setGeometry(QRect(470, 30, 113, 20));
        label = new QLabel(Widget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(430, 30, 31, 16));
        label_2 = new QLabel(Widget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(430, 60, 31, 16));
        ledtPort = new QLineEdit(Widget);
        ledtPort->setObjectName(QStringLiteral("ledtPort"));
        ledtPort->setGeometry(QRect(470, 60, 113, 20));
        btnConnect = new QPushButton(Widget);
        btnConnect->setObjectName(QStringLiteral("btnConnect"));
        btnConnect->setGeometry(QRect(480, 90, 75, 23));
        btnDisconnect = new QPushButton(Widget);
        btnDisconnect->setObjectName(QStringLiteral("btnDisconnect"));
        btnDisconnect->setGeometry(QRect(480, 120, 75, 23));
        btnSend = new QPushButton(Widget);
        btnSend->setObjectName(QStringLiteral("btnSend"));
        btnSend->setGeometry(QRect(480, 320, 75, 23));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "EasyTcp - Client", Q_NULLPTR));
        ledtHost->setText(QApplication::translate("Widget", "127.0.0.1", Q_NULLPTR));
        label->setText(QApplication::translate("Widget", "\345\234\260\345\235\200", Q_NULLPTR));
        label_2->setText(QApplication::translate("Widget", "\347\253\257\345\217\243", Q_NULLPTR));
        ledtPort->setText(QApplication::translate("Widget", "6948", Q_NULLPTR));
        btnConnect->setText(QApplication::translate("Widget", "\350\277\236\346\216\245", Q_NULLPTR));
        btnDisconnect->setText(QApplication::translate("Widget", "\346\226\255\345\274\200", Q_NULLPTR));
        btnSend->setText(QApplication::translate("Widget", "\345\217\221\351\200\201", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H

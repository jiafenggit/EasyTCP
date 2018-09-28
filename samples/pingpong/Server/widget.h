#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include "EasyTcp.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void textNeedPrint(QTextEdit* control, QString str);

private:
    void whenConnected(EasyTCP::IConnection *con);
    void whenDisconnected(EasyTCP::IConnection *con);
    void whenBufferSent(EasyTCP::IConnection*, EasyTCP::AutoBuffer data);
    void whenBufferReceived(EasyTCP::IConnection*, EasyTCP::AutoBuffer data);

private slots:
    void open();
    void close();

    void printText(QTextEdit* control, QString str);

private:
    Ui::Widget *ui;
    EasyTCP::IServerPtr m_server;
};

#endif // WIDGET_H

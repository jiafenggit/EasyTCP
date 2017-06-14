#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include "EasyTcpServer.h"

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
    void whenConnected(EasyTcp::Connection *con);
    void whenDisconnected(EasyTcp::Connection *con);
    void whenBufferSent(EasyTcp::Connection*, EasyTcp::AutoBuffer data);
    void whenBufferReceived(EasyTcp::Connection*, EasyTcp::AutoBuffer data);

private slots:
    void open();
    void close();

    void printText(QTextEdit* control, QString str);

private:
    Ui::Widget *ui;
    EasyTcp::SPTRServer m_server;
};

#endif // WIDGET_H

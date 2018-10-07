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
    void whenConnected(EasyTCP::IConnection*);
    void whenConnectFailed(EasyTCP::IConnection*, int err);
    void whenDisconnected(EasyTCP::IConnection*);
    void whenBufferSent(EasyTCP::IConnection*, EasyTCP::AutoBuffer data);
    void whenBufferReceived(EasyTCP::IConnection*, EasyTCP::AutoBuffer data);

private slots:
    void connect();
    void disconnect();
    void send();

    void printText(QTextEdit* control, QString str);

private:
    Ui::Widget *ui;
    EasyTCP::IClientPtr m_client;
    EasyTCP::AutoBuffer m_recvBuf;
};

#endif // WIDGET_H

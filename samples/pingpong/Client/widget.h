#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include "EasyTcpClient.h"
#include "../../test_config.h"

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
    void whenConnected(EasyTcp::Connection*);
    void whenConnectFailed(EasyTcp::Connection*, int err);
    void whenDisconnected(EasyTcp::Connection*);
    void whenBufferSent(EasyTcp::Connection*, EasyTcp::AutoBuffer data);
    void whenBufferReceived(EasyTcp::Connection*, EasyTcp::AutoBuffer data);

private slots:
    void connect();
    void disconnect();
    void send();

    void printText(QTextEdit* control, QString str);

private:
    Ui::Widget *ui;
    EasyTcp::SPTRClient m_client;
    Count m_count;
};

#endif // WIDGET_H

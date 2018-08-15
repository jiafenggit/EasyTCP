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
    void whenConnected(EasyTcp::IConnection *con);
    void whenDisconnected(EasyTcp::IConnection *con);
    void whenBufferSent(EasyTcp::IConnection*, EasyTcp::AutoBuffer data);
    void whenBufferReceived(EasyTcp::IConnection*, EasyTcp::AutoBuffer data);

private slots:
    void open();
    void close();

    void printText(QTextEdit* control, QString str);

private:
    Ui::Widget *ui;
    EasyTcp::IServerPtr m_server;
};

#endif // WIDGET_H

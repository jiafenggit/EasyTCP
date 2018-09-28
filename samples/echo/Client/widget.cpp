#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDateTime>
#include "../../test_config.h"

using namespace std::placeholders;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QObject::connect(ui->btnSend, SIGNAL(clicked(bool)), this, SLOT(send()));
    QObject::connect(ui->btnConnect, SIGNAL(clicked(bool)), this, SLOT(connect()));
    QObject::connect(ui->btnDisconnect, SIGNAL(clicked(bool)), this, SLOT(disconnect()));
    QObject::connect(this, SIGNAL(textNeedPrint(QTextEdit*,QString)),
            this, SLOT(printText(QTextEdit*,QString)));

    m_client = EasyTCP::Client::create();
    if (!m_client.get())
    {
        QMessageBox::critical(NULL, "初始化失败", "初始化 client 失败");
        exit(1);
        return;
    }

    m_client->onConnected = std::bind(&Widget::whenConnected, this, _1);
    m_client->onConnectFailed = std::bind(&Widget::whenConnectFailed, this, _1, _2);
    m_client->onDisconnected = std::bind(&Widget::whenDisconnected, this, _1);
    m_client->onBufferSent = std::bind(&Widget::whenBufferSent, this, _1, _2);
    m_client->onBufferReceived = std::bind(&Widget::whenBufferReceived, this, _1, _2);
}

Widget::~Widget()
{
    m_client.reset();
    delete ui;
}

void Widget::whenConnected(EasyTCP::IConnection *)
{
    emit textNeedPrint(ui->txtedtMsg, "已连接");
    m_client->enableKeepalive();
    m_client->setSendBufferSize(128 * 1024);
    m_client->setReceiveBufferSize(128 * 1024);
    m_client->setLinger(1, 0);

    EasyTCP::AutoBuffer buf;
    buf.reset(TEST_DEFAULT_DATA_SIZ);
    if (!buf.size())
    {
        emit textNeedPrint(ui->txtedtMsg, "接收数据，内存不足");
        return;
    }

    if(!m_client->recv(buf))
        m_client->disconnect();
}

void Widget::whenConnectFailed(EasyTCP::IConnection *, int err)
{
    emit textNeedPrint(ui->txtedtMsg, "连接失败，错误 " + QString::number(err) + "");
}

void Widget::whenDisconnected(EasyTCP::IConnection *)
{
    emit textNeedPrint(ui->txtedtMsg, "已断开");
}

void Widget::whenBufferSent(EasyTCP::IConnection *, EasyTCP::AutoBuffer data)
{
    QString str;
    str.append(" 已发送:")
        .append(data.data())
        .append("");

    emit textNeedPrint(ui->txtedtMsg, str);
}

void Widget::whenBufferReceived(EasyTCP::IConnection *, EasyTCP::AutoBuffer data)
{
    QString str;
    str.append("已接收:");

    if (data.size() != TEST_DEFAULT_DATA_SIZ)
    {
        str.append("接收到异常数据, 期望接收到 " +  QString::number(TEST_DEFAULT_DATA_SIZ)
                   + " 字节，实际字节 " + QString::number(data.size()));
    }
    else
    {
        str.append(data.data());

        EasyTCP::AutoBuffer buf;
        buf.reset(TEST_DEFAULT_DATA_SIZ);
        if (!buf.size())
        {
            str.append("\n接收数据，内存不足");
        }
        else
        {
            if(!m_client->recv(buf))
            {
                str.append("\nrecv失败");
                m_client->disconnect();
            }
        }
    }

    emit textNeedPrint(ui->txtedtMsg, str);
}

void Widget::connect()
{
    QString host;
    unsigned short port;

    if (ui->ledtHost->text().isEmpty())
    {
        QMessageBox::information(NULL, "提示", "地址不能为空");
        return;
    }

    host = ui->ledtHost->text();

    if (ui->ledtPort->text().isEmpty())
    {
        QMessageBox::information(NULL, "提示", "端口不能为空");
        return;
    }

    try
    {
        port = ui->ledtPort->text().toShort();
    }
    catch(...)
    {
        QMessageBox::information(NULL, "提示", "端口不是有效的数字");
        return;
    }

    if (!m_client->connect(host.toStdString(), port))
    {
        ui->txtedtMsg->append("连接失败");
    }
}

void Widget::disconnect()
{
    if(!m_client->disconnect())
    {
        ui->txtedtMsg->append("断开连接失败");
    }
}

void Widget::send()
{
    EasyTCP::AutoBuffer buf;
    std::string str = ui->txtedtInput->toPlainText().toStdString();

    if (str.empty())
    {
        return;
    }

    if (str.length() >= TEST_DEFAULT_DATA_SIZ)
    {
        QMessageBox::critical(NULL, "提示", "文本长度超过 " + QString::number(TEST_DEFAULT_DATA_SIZ) + " 字符");
        return;
    }

    buf.reset(TEST_DEFAULT_DATA_SIZ);
    if (!buf.data())
    {
        QMessageBox::critical(NULL, "提示", "内存不足");
        return;
    }

    memcpy(buf.data(), str.c_str(), str.length() + 1);

    if (!m_client->send(buf))
    {
        ui->txtedtMsg->append("发送失败");
        m_client->disconnect();
    }
    else
    {
        ui->txtedtInput->clear();
    }
}

void Widget::printText(QTextEdit *control, QString str)
{
    control->append(QDateTime::currentDateTime().toString());
    control->append(str.append("\n"));
}

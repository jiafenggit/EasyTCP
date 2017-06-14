#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QTime>
#include "../../test_config.h"

using namespace std::placeholders;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    QObject::connect(ui->btnOpen, SIGNAL(clicked(bool)), this, SLOT(open()));
    QObject::connect(ui->btnClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    QObject::connect(this, SIGNAL(textNeedPrint(QTextEdit*,QString)),
            this, SLOT(printText(QTextEdit*,QString)));

    m_server = EasyTcp::Server::create();
    if (!m_server.get())
    {
        QMessageBox::critical(NULL, "初始化失败", "初始化 server 失败");
        exit(1);
        return;
    }

    m_server->onConnected = std::bind(&Widget::whenConnected, this, _1);
    m_server->onDisconnected = std::bind(&Widget::whenDisconnected, this, _1);
    m_server->onBufferSent = std::bind(&Widget::whenBufferSent, this, _1, _2);
    m_server->onBufferReceived = std::bind(&Widget::whenBufferReceived, this, _1, _2);
}

Widget::~Widget()
{
    m_server.reset();;
    delete ui;
}

void Widget::whenConnected(EasyTcp::Connection *con)
{
    QString str;

    str.append(con->peerIP().c_str())
        .append(":")
        .append(QString::number(con->peerPort()))
        .append(" 已连接");

    emit textNeedPrint(ui->txtedtMsg, str);

    con->enableKeepalive();
    con->setSendBufferSize(128 * 1024);
    con->setReceiveBufferSize(128 * 1024);
    con->setLinger(1, 0);

    Count *pCount = new Count();
    memset(pCount, 0, sizeof(Count));
    con->bindUserdata(pCount);

    EasyTcp::AutoBuffer buf;
    buf.reset(TEST_DEFAULT_DATA_SIZ);
    if (!buf.size())
    {
        emit textNeedPrint(ui->txtedtMsg, "接收数据，内存不足");
        return;
    }

    if(!con->recv(buf))
    {
        con->disconnect();
    }
}

void Widget::whenDisconnected(EasyTcp::Connection *con)
{
    QString str;
    Count *pCount = (Count *)(con->userdata());

    str.append(con->peerIP().c_str())
        .append(":")
        .append(QString::number(con->peerPort()))
        .append(" 已断开\n")
        .append("共发送" + QString::number(pCount->countWrite) + "次, "
               + QString::number(pCount->bytesWrite) + " 字节, 接收 "
               + QString::number(pCount->countRead) + "次, "
               + QString::number(pCount->bytesRead) + " 字节");

    delete pCount;
    emit textNeedPrint(ui->txtedtMsg, str);
}

void Widget::whenBufferSent(EasyTcp::Connection *con, EasyTcp::AutoBuffer data)
{
    Count *pCount = (Count *)(con->userdata());
    pCount->countWrite++;
    pCount->bytesWrite += data.size();
}

void Widget::whenBufferReceived(EasyTcp::Connection *con, EasyTcp::AutoBuffer data)
{
    QString str;
    Count *pCount = (Count *)(con->userdata());

    str.append(con->peerIP().c_str())
        .append(":")
        .append(QString::number(con->peerPort()))
        .append(":");

    if (data.size() != TEST_DEFAULT_DATA_SIZ)
    {
        str.append("接收到异常数据, 期望接收到 " +  QString::number(TEST_DEFAULT_DATA_SIZ)
                   + " 字节，实际字节 " + QString::number(data.size()));
        emit textNeedPrint(ui->txtedtMsg, str);
    }
    else
    {
        EasyTcp::AutoBuffer buf;
        buf.reset(TEST_DEFAULT_DATA_SIZ);
        if (!buf.size())
        {
            str.append("接收数据，内存不足");
            emit textNeedPrint(ui->txtedtMsg, str);
        }
        else
        {
            pCount->countRead++;
            pCount->bytesRead += data.size();

            if(!con->send(data) || !con->recv(buf))
            {
                con->disconnect();
            }
        }
    }
}

void Widget::open()
{
    unsigned short port;

    try
    {
        port = ui->ledtPort->text().toShort();
    }
    catch(...)
    {
        QMessageBox::information(NULL, "提示", "端口不是有效的数字");
        return;
    }

    if (!m_server->open(port))
    {
        ui->txtedtMsg->append("开启失败");
    }
    else
    {
        ui->txtedtMsg->append("已开启");
    }
}

void Widget::close()
{
    m_server->close();
    ui->txtedtMsg->append("已关闭");
}

void Widget::printText(QTextEdit *control, QString str)
{
    control->append(QDateTime::currentDateTime().toString());
    control->append(str.append("\n"));
}

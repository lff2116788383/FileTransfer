#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#pragma execution_character_set("utf-8")
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,8888);
    setWindowTitle("服务器端口为：8888");

    ui->buttonFile->setEnabled(false);
    ui->buttonSend->setEnabled(false);

    connect(tcpServer,&QTcpServer::newConnection,[=](){
        //取出建立好连接的套接字
        tcpSocket=tcpServer->nextPendingConnection();

        QString ip=tcpSocket->peerAddress().toString();
        quint16 port=tcpSocket->peerPort();

        QString str=QString("[%1#%2] 成功连接").arg(ip).arg(port);
        ui->textEdit->setText(str);

        ui->buttonFile->setEnabled(true);
    });

    connect(&timer,&QTimer::timeout,[=](){
        timer.stop();
        sendData();
    });

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_buttonFile_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(this,"open","../");
    if(false==filepath.isEmpty())
    {
        fileName.clear();
        fileSize=0;

        //获取文件信息
        QFileInfo info(filepath);
        fileName=info.fileName();//获取文件名
        fileSize=info.size();//获取文件大小

        sendSize=0;


        //只读方式打开文件
        //指定文件的名字
        file.setFileName(filepath);

        bool isOk = file.open(QIODevice::ReadOnly);
        if(isOk==false)
        {
            qDebug()<<"只读方式打开文件失败！";
        }
        ui->textEdit->append(filepath);
        ui->buttonFile->setEnabled(false);
        ui->buttonSend->setEnabled(true);

    }
    else
    {
        qDebug()<<"选择文件出错！";
    }
}

void Widget::on_buttonSend_clicked()
{
    //先发送文件头信息  文件名##文件大小
    QString head = QString("%1##%2").arg(fileName).arg(fileSize);
    //发送真正的文件信息
    qint64 len =tcpSocket->write(head.toUtf8());
    if(len>0)//头部信息发送成功
    {
        timer.start(20);
    }
    else
    {
        qDebug()<<"头部信息发送失败！";
        file.close();
        ui->buttonFile->setEnabled(true);
        ui->buttonSend->setEnabled(false);

    }
}

void Widget::sendData()
{
    qint64 len = 0;
    do{
        char buf[4*1024]={0};
        len = 0;
        len = file.read(buf,sizeof(buf));
        len=tcpSocket->write(buf,len);
        sendSize+=len;

    }
    while(len>0);

    //是否文件发送完毕
    if(sendSize==fileSize)
    {
        ui->textEdit->append("文件发送完毕");
        file.close();

        tcpSocket->disconnectFromHost();
        tcpSocket->close();

    }
}

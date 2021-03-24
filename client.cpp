#include "client.h"
#include "ui_client.h"
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#pragma execution_character_set("utf-8")
Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    setWindowTitle("客户端");
    tcpSocket=new QTcpSocket(this);
    isStart=true;
    connect(tcpSocket,&QTcpSocket::readyRead,[=](){
        //取出已经接收的内容
         QByteArray buf = tcpSocket->readAll();

        if(isStart)
        {
            isStart=false;

            //初始化
            fileName=QString(buf).section("##",0,0);
            fileSize=QString(buf).section("##",1,1).toInt();
            recvSize=0;

            file.setFileName(fileName);
            bool isOk=file.open(QIODevice::WriteOnly);
            if(false==isOk)
            {
                qDebug()<<"WriteOnly Error";
            }
        }
        else
        {
            qint64 len=file.write(buf);
            recvSize+=len;
            if(recvSize==fileSize)
            {
                file.close();

                QMessageBox::information(this,"完成","文件接收完毕");
                tcpSocket->disconnectFromHost();
                tcpSocket->close();
            }


        }

    });

}

Client::~Client()
{
    delete ui;
}

void Client::on_pushButton_clicked()
{
   QString ip=ui->lineEditIP->text();
   quint16 port= ui->lineEditPort->text().toInt();

   tcpSocket->connectToHost(QHostAddress(ip),port);
}

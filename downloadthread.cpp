#include "downloadthread.h"
#include <QThread>
#include <QDebug>
#include <QSqlQuery>
#include <mainwindow.h>
#include <QSqlError>
DownloadThread::DownloadThread(QObject *parent) : QObject(parent)
{
    Set_Database();
}
DownloadThread::~DownloadThread()
{
    timer->stop();
    timer->deleteLater();
}

//QMutex Iot::mutex;

void DownloadThread::onCreateTimer()
{
    //关键点：在子线程中创建QTimer的对象
    timer = new QTimer();
    //每10秒刷新一次页面
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    //单次定时器，首次加载IOT传递初始数据
    QTimer::singleShot(0,this, SLOT(onTimeout()));
    //timer->stop();
}
//在子线程中开启定时器函数，用于主线程操纵子线程（定时器只能在创建函数中操作）
void DownloadThread::timerStartOrStop(int t){
    if(t==0){
        timer->start();
    }else if(t==1){
        timer->stop();
    }
}
//定时器插槽函数
void DownloadThread::Set_Database(){

    //本地数据库开启代码
    if (QSqlDatabase::contains("download2")) {
        dblocal = QSqlDatabase::database("download2");
    } else {
        dblocal = QSqlDatabase::addDatabase("QODBC","download2");
    }
    dblocal.setHostName("127.0.0.1");
    dblocal.setPort(3306);
    dblocal.setDatabaseName("Mydesign");
    dblocal.setUserName("root");
    dblocal.setPassword("200243");
    bool ok = dblocal.open();
    if (ok){
        qDebug("下载：本地数据库连接成功！");
    }
    else {
        qDebug("下载：本地数据库连接失败！");
        qDebug()<<"error open database because"<<dblocal.lastError().text();
    }
}

void DownloadThread::onTimeout()
{
    MainWindow::mutex.lock();
    QSqlQuery query(dblocal);
    //int ok;
    bool first=true;
    qDebug()<<"下载子线程运行中!";
    //QSqlQuery queryl(*dblocal);
    query.exec("select * from HUMIandTEMP1 order by timeset desc;");
    first=true;
    while(query.next()){
        if(first){
            first=false;
            time1=query.value(0).toString();
            data1=query.value(2).toString();
        }
    }

    query.exec("delete from HUMIandTEMP2");
    query.exec("select * from HUMIandTEMP2 order by timeset desc;");
    first=true;
    while(query.next()){
        if(first){
            first=false;
            time2=query.value(0).toString();
            data2=query.value(2).toString();
        }
    }
    //像主线程传递显示数据（iot.cpp第52行）
    emit sendText(time1,data1,time2,data2);
    //db->close();
    MainWindow::mutex.unlock();
}

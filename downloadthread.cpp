#include "downloadthread.h"
#include <QThread>
#include <QDebug>
#include <QSqlQuery>
#include <mainwindow.h>
#include <QSqlError>

DownloadThread::DownloadThread(int sensornum,QString *num,QObject *parent) : QObject(parent)
{
    Set_Database();
    sensorn=sensornum;
    //qDebug()<<*(num+1);
    for(int i=0;i<sensorn;i++){
        n[i]=*(num+i);
    }
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
    //每1秒刷新一次页面
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    //单次定时器，首次加载IOT传递初始数据
    //QTimer::singleShot(0,this, SLOT(onTimeout()));
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
    dblocal.setHostName("120.25.3.98");
    dblocal.setPort(3306);
    dblocal.setDatabaseName("Mydesign1");
    dblocal.setUserName("IonCt");
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
    int ok;
    bool first=true;
    qDebug()<<"下载子线程运行中!";

    for(int i=0;i<sensorn;i++){
        QString queryString = "select * from %1 order by timeset desc;";
        queryString = queryString.arg(n[i]);
        ok=query.exec(queryString);
        if (!ok) {
            qDebug() << "check data into local database error: " << query.lastError().text();

        }
        else{
            //qDebug()<< ok;
            while(query.next()){
                if(first){
                    first=false;
                    time[i]=query.value(0).toString();
                    data[i]=query.value(1).toString();
                }
            }
        }
        first=true;
    }
    emit sendText(time,data);
    MainWindow::mutex.unlock();
}

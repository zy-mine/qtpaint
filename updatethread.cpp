#include "updatethread.h"
#include <QThread>
#include <QDebug>
#include <QSqlQuery>
#include <mainwindow.h>
#include <QSqlError>
#include <QTime>
#define RAND_MAX 0x7fff



UpdateThread::UpdateThread(int sensornum,QString *num,QObject *parent) : QObject(parent)
{
    if (QSqlDatabase::contains("update")) {
        db = QSqlDatabase::database("update");
    } else {
        db=QSqlDatabase::addDatabase("QODBC","update");
    }
    db.setHostName("120.25.3.98");
    db.setPort(3306);
    db.setDatabaseName("Mydesign1");
    db.setUserName("IonCt");
    db.setPassword("200243");
    bool ok = db.open();
    if (ok){
        qDebug("数据模拟：阿里云数据库连接成功！");
    }
    else {
        qDebug("数据模拟：阿里云数据库连接失败！");
        qDebug()<<"error open database because"<<db.lastError().text();
    }

    sensorn=sensornum;
    //qDebug()<<*(num+1);
    for(int i=0;i<sensorn;i++){
        n[i]=*(num+i);
    }
}
UpdateThread::~UpdateThread()
{
    timer->stop();
    timer->deleteLater();
}

//QMutex Iot::mutex;

void UpdateThread::onCreateTimer()
{
    //关键点：在子线程中创建QTimer的对象
    timer = new QTimer();
    //设置每7秒模拟一次数据
    timer->setInterval(10000);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    //timer->start();
}
void UpdateThread::timerStartOrStop(int t){
    if(t==0){
        timer->start();
    }else if(t==1){
        timer->stop();
    }
}

//用于保留两位小数
QString stage(double v, int precision) //precision为需要保留的精度，2位小数变为2
{
    return (v >= 0.00 ? QString::number(v, 'f', precision) : "");
}
//随机函数
int RandomNum(int min_num, int max_num)
{
    if (min_num > max_num)
    {
        int tmp_num = min_num;
        min_num = max_num;
        max_num = tmp_num;
    }
    int interval_num = max_num - min_num;
    if (interval_num <= 0)
    {
        return 0;
    }
    if (interval_num < RAND_MAX)
    {
        return min_num + (rand() % interval_num);
    }
    else
    {
        return min_num + int(((rand() % RAND_MAX) * 1.0 / RAND_MAX) * interval_num);
    }
}
void UpdateThread::onTimeout()
{
    MainWindow::mutex.lock();
    qDebug()<<"模拟数据子线程运行中！";
    QDateTime time = QDateTime::currentDateTime();
    QString sTime = time.toString("yyyy-MM-dd hh:mm:ss");//0-55摄氏度，100%湿度
    QSqlQuery query(db);
    int ok;
    bool first=true;
    double d;
    QString queryString;
    for(int i=0;i<sensorn;i++){
        //读取历史最新数据
        queryString = "select * from %1 order by timeset desc;";
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
                    //time[i]=query.value(0).toString();
                    data[i]=query.value(1).toString();
                }
            }
        }
        first=true;
        d=data[i].toDouble();
        //模拟最新数据
        QString pop=stage(RandomNum(d*60,(1-d)*40+100*d)*0.01,2);
        queryString = "INSERT INTO %1 (timeset,data) VALUES ('%2','%3')";
        queryString = queryString.arg(n[i]).arg(sTime).arg(pop);
        ok=query.exec(queryString);
        if(!ok){
            qDebug() << "check data into local database error: " << query.lastQuery();
        }
        else{
            qDebug() <<"新数据为"<<n[i]<<"  "<<pop;
        }
    }

    // double light =RandomNum(0,7000000)*0.01;;//lux光照强度，7w
    // //qDebug()<<light;
    // if(status5==0){
    //     light=0;
    // }
    // light==0?statu="不正常":statu="正常";
    //                                        query.prepare("INSERT INTO Light(timeset,light,status) "
    //                                                      "VALUES(:timeset,:light,:status)");
    // query.bindValue(":timeset", sTime);
    // query.bindValue(":light", stage(light,2));
    // query.bindValue(":status",statu);
    // query.exec();
    MainWindow::mutex.unlock();
}

#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QObject>
#include <QTimer>
#include <QSqlDatabase>
class DownloadThread: public QObject
{
    Q_OBJECT
public:
    explicit DownloadThread(QObject *parent = nullptr);
    ~DownloadThread();
    void Set_Database();


public slots:
    void onCreateTimer();
    void onTimeout();
    void timerStartOrStop(int t);
signals:
    void sendText(QString,QString,QString,QString,QString,QString,QString,QString);
private:
    QTimer* timer = nullptr;
    QSqlDatabase dblocal;
    QString time1="";
    QString data1="";

    QString time2="";
    QString data2="";
    QString time3="";
    QString data3="";
    QString time4="";
    QString data4="";
};

#endif // DOWNLOADTHREAD_H

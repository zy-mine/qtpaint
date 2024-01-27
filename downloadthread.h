#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QObject>
#include <QTimer>
#include <QSqlDatabase>

class DownloadThread: public QObject
{
    Q_OBJECT
public:
    explicit DownloadThread(int sensornum,QString num[100],QObject *parent = nullptr);
    ~DownloadThread();
    void Set_Database();


public slots:
    void onCreateTimer();
    void onTimeout();
    void timerStartOrStop(int t);
signals:
    void sendText(QString *,QString *);
private:
    QTimer* timer = nullptr;
    QSqlDatabase dblocal;
    QString time[100],data[100];
    int sensorn;
    QString n[100];
};

#endif // DOWNLOADTHREAD_H

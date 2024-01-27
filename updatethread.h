#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H

#include <QObject>
#include <QTimer>
#include <QSqlDatabase>

class UpdateThread: public QObject
{
    Q_OBJECT
public:
    explicit UpdateThread(int sensornum,QString num[100],QObject *parent = nullptr);
    ~UpdateThread();
public slots:
    void onCreateTimer();
    void onTimeout();
    void timerStartOrStop(int t);
private:
    QTimer* timer = nullptr;
    QSqlDatabase db;
    int sensorn;
    QString n[100];
    QString time[100],data[100];
};

#endif // UPDATETHREAD_H

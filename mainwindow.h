#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QMutex>
#include <downloadthread.h>

#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QMutex mutex;
    void Set_Pic();
private slots:
    void update(QString time1,QString data1,QString time2,QString data2,QString time3,QString data3,QString time4,QString data4);
signals:
    void sendData(int);

private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot;
    QSqlDatabase dblocal;
    QTimer timer;
    DownloadThread *download;
    QThread downloadTh;
    QCPColorMap *colorMap;
    int nx,ny;
    double x, y, z;
    QString num1,num2;
    double x1,y1,x2,y2,x3,x4,y3,y4;
};
#endif // MAINWINDOW_H

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
    int sensornum;
private slots:
    void update(QString *time,QString *data);
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
    QString num1[100];
    double x1,y1,x2,y2,x3,x4,y3,y4;
    double d1,d2,d3,d4;
    double xg[100],yg[100];//存储各个传感器位置信息
    double u,v;

};
#endif // MAINWINDOW_H

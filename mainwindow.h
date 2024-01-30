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
#include <updatethread.h>
#include <switchbtn.h>
#include <QMouseEvent>

#include "qcustomplot.h"
extern int currenttype;
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
    int sensortypenum;
    //int currenttype;
private slots:
    void update(QString *time,QString *data);
    void on_pushButton_clicked();
    //void on_pushButton_2_clicked();
    void on_update_button_clicked();
    void mouseMoveEvent(QMouseEvent *mouseEvent);

    void on_comboBox_currentIndexChanged(int index);

public slots:
    void Slot1();
    //void mouseRelease(QMouseEvent *event);
signals:
    void sendData(int);
    void sendData1(int);
private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot;
    QSqlDatabase dblocal;
    QTimer timer;
    int timer1=0;
    int timer3_timeout=0;
    bool mathe=false;

    QCPItemText *textLabel;//显示鼠标当前位置离子浓度

    bool firstcg_combobox=false;
    DownloadThread *download;
    SwitchBtn *switchbtn;
    QThread downloadTh;

    UpdateThread *updateth;
    QThread updateTh;

    QCPColorMap *colorMap;
    int nx,ny;
    double x, y, z;
    QString num1[100],type1[20];
    double x1,y1,x2,y2,x3,x4,y3,y4;
    double d1,d2,d3,d4;
    double xg[100],yg[100];//存储各个传感器位置信息
    double u,v;
    QCPColorScale *colorScale;


};
#endif // MAINWINDOW_H

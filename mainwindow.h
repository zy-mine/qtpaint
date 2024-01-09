#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlDatabase>
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
    void update(QString time1,QString data1,QString time2,QString data2);

private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot;
    QSqlDatabase dblocal;
    QTimer timer;
    DownloadThread *download;
    QThread downloadTh;
    QCPColorMap *colorMap;
    int nx,ny;
};
#endif // MAINWINDOW_H

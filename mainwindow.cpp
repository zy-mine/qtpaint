#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <gsl/gsl_spline.h>
#include <gsl/gsl_spline2d.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_interp2d.h>

QMutex MainWindow::mutex;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        //qDebug()<<"error open database because"<<dblocal.lastError().text();
    }
    QSqlQuery query(dblocal);
    query.exec("select * from sensor order by yloc asc,xloc asc;");
    //bool first=true;
    for(sensornum=0;query.next();sensornum++){
         num1[sensornum]=query.value(0).toString();
         xg[sensornum]=query.value(2).toString().toDouble();
         yg[sensornum]=query.value(3).toString().toDouble();
         //qDebug()<<num1[sensornum]<<xg[sensornum]<<yg[sensornum];

    }

    switchbtn=ui->widget;
    connect(switchbtn,SIGNAL(btnChanged()),this,SLOT(Slot1()));

    download = new DownloadThread(sensornum,num1);
    download->moveToThread(&downloadTh);
    connect(&downloadTh,SIGNAL(started()),download,SLOT(onCreateTimer()));
    connect(&downloadTh,&QThread::finished,download,&QObject::deleteLater);
    downloadTh.start();
    //连接子线程传来的显示数据
    connect(download,SIGNAL(sendText(QString *,QString *)),this,SLOT(update(QString*,QString*)));//信号
    connect(this,SIGNAL(sendData(int)),download,SLOT(timerStartOrStop(int)));




    //ui->widget=customPlot;
    customPlot=ui->custom;
    //setCentralWidget(customPlot);
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");

    // set up the QCPColorMap:
    colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
    nx = 200;
    ny = 200;
    this->Set_Pic();


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::Set_Pic(){
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions 并张成坐标范围-4。键(x)和值(y)维度均为4

     customPlot->xAxis->setLabel("经度");
     customPlot->yAxis->setLabel("纬度");
    // colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // // rescale the key (x) and value (y) axes so the whole color map is visible:
    // //重新缩放键(x)和值(y)轴，使整个彩色地图可见:
    // customPlot->rescaleAxes();
    colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default) 刻度应该是带有刻度/轴标签的竖条(实际上atRight已经是默认值了)
    colorScale->setDataRange(QCPRange(0,101));
    colorMap->setColorScale(colorScale); // associate the color map with the color scale 将颜色映射与颜色比例关联起来
    colorScale->axis()->setLabel("离子浓度%");
    // 将颜色映射的颜色渐变设置为预设之一
    // 我们也可以创建一个QCPColorGradient实例，并在渐变中添加自己的颜色。
    colorMap->setGradient(QCPColorGradient::gpJet);
}
//void MainWindow::update(QString time1,QString data1,QString time2,QString data2,QString time3,QString data3,QString time4,QString data4)
void MainWindow::update(QString *time,QString *data){
    double d[100];
    for(int i=0;i<sensornum;i++){
        d[i]=data[i].toDouble();
        //qDebug()<<"d"<<d[i];
    }


    //控制最大为17，传入数据data默认在0-1之间。
    //qDebug()<<"x,y"<<x1<<y1<<x2<<y2<<x3<<y3<<x4<<y4<<d1<<d2<<d3<<d4;
    //double distance,min_distance[4]={100};
    //int min_sensor[4]={0};
    //bool first=true;

    if(mathe){
        // 创建 GSL 插值所需的变量
        double xa[5]={-4,-2,0,2,4};
        //double xa[5]={0,2,4,6,8};
        double ya[5]={-4,-2,0,2,4};
        //double ya[5]={0,2,4,6,8};
        gsl_interp_accel *xacc = gsl_interp_accel_alloc();
        gsl_interp_accel *yacc = gsl_interp_accel_alloc();
        const size_t nx1 = sizeof(xa) / sizeof(double); /* x grid points */
        const size_t ny1 = sizeof(ya) / sizeof(double); /* y grid points */

        double *za=(double *)malloc(nx1*ny1*sizeof(double));
        gsl_spline2d *spline = gsl_spline2d_alloc(gsl_interp2d_bicubic, nx1, ny1);
        for(int j=0;j<ny1;j++){
            for(int i=0;i<nx1;i++){
                gsl_spline2d_set(spline,za,i,j,d[j*nx1+i]);
                //qDebug()<<xa[i]<<ya[j]<<j*nx1+i;
            }
        }
        // 初始化 x 和 y 方向上的插值
        gsl_spline2d_init(spline, xa, ya, za, nx1, ny1);
        colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
        colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4));

        /* interpolate N values in x and y and print out grid for plotting */
        for (int i = 0; i < nx; ++i)
        {
            //double xi = i/nx*8-4;

            for (int j = 0; j < ny; ++j)
            {
                colorMap->data()->cellToCoord(i, j, &x, &y);
                //double yj = j/ny*8-4;
                double zij = gsl_spline2d_eval(spline, x, y, xacc, yacc);
                //qDebug()<<zij;
                //printf("%f %f %f\n", xi, yj, zij);

                //colorMap->data()->cellToCoord(xi, yj, &x, &y);

                colorMap->data()->setCell(i, j, zij*100);
            }
            //printf("\n");
        }

    }

    //注释1
    else if(!mathe){
        int p=qSqrt(sensornum);
        colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
        colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4));
        for (int i=1;i<p;i++){
            for (int j=0;j<p-1;j++){
                x1=xg[(i-1)*p+j];x2=xg[(i-1)*p+j+1];x3=xg[i*p+j];x4=xg[i*p+j+1];
                y1=yg[(i-1)*p+j];y2=yg[(i-1)*p+j+1];y3=yg[i*p+j];y4=yg[i*p+j+1];
                d1=d[(i-1)*p+j]*100;
                d2=d[(i-1)*p+j+1]*100;
                d3=d[i*p+j]*100;
                d4=d[i*p+j+1]*100;
                //qDebug()<<"x,y"<<x1<<y1<<x2<<y2<<x3<<y3<<x4<<y4<<d1<<d2<<d3<<d4;
                for (int xIndex=(x1+4)/8*nx; xIndex<(x2+4)/8*nx; ++xIndex)
                {
                    for (int yIndex=(y1+4)/8*ny; yIndex<(y3+4)/8*ny; ++yIndex)
                    {
                        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
                        u=(x-x1)/(x2-x1);
                        v=(y-y1)/(y3-y1);
                        z=d1*(1-u)*(1-v)+d2*(1-v)*u+d3*(1-u)*v+d4*u*v;
                        colorMap->data()->setCell(xIndex, yIndex, z);
                        //qDebug()<<"x,y"<<x<<y<<"      "<<x1<<y1<<x2<<y2<<x3<<y3<<x4<<y4<<d1<<d2<<d3<<u<<v<<z;
                    }
                }
            }
        }
    }








    //重新缩放数据维度(颜色)，使所有数据点都位于颜色梯度显示的跨度中:
    colorMap->rescaleDataRange();
    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    //确保轴矩形和颜色比例同步他们的底部和顶部边距(所以他们对齐):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    // rescale the key (x) and value (y) axes so the whole color map is visible:
    //重新缩放键(x)和值(y)轴，使整个彩色地图可见:
    customPlot->rescaleAxes();
    customPlot->replot();
}



void MainWindow::on_pushButton_clicked()
{
    connect(this,SIGNAL(sendData(int)),download,SLOT(timerStartOrStop(int)));
    //通过子线程开启模拟数据定时器
    emit sendData(timer1);

    mathe=false;
}

void MainWindow::Slot1(){
    connect(this,SIGNAL(sendData(int)),download,SLOT(timerStartOrStop(int)));
    //通过子线程开启模拟数据定时器
    emit sendData(timer1);
    mathe=!mathe;
}

void MainWindow::on_pushButton_2_clicked()
{
    connect(this,SIGNAL(sendData(int)),download,SLOT(timerStartOrStop(int)));
    //通过子线程开启模拟数据定时器
    emit sendData(timer1);
    //timer1=0;
    mathe=true;
}




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
    dblocal.setHostName("127.0.0.1");
    dblocal.setPort(3306);
    dblocal.setDatabaseName("Mydesign");
    dblocal.setUserName("root");
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
         qDebug()<<num1[sensornum]<<xg[sensornum]<<yg[sensornum];

    }

    // query.next();
    // num1=query.value(0).toString();
    // x1=query.value(2).toString().toDouble();
    // y1=query.value(3).toString().toDouble();

    // query.next();
    // num2=query.value(0).toString();
    // x2=query.value(2).toString().toDouble();
    // y2=query.value(3).toString().toDouble();

    // query.next();
    // num2=query.value(0).toString();
    // x3=query.value(2).toString().toDouble();
    // y3=query.value(3).toString().toDouble();

    // query.next();
    // num2=query.value(0).toString();
    // x4=query.value(2).toString().toDouble();
    // y4=query.value(3).toString().toDouble();

    download = new DownloadThread(sensornum,num1);
    download->moveToThread(&downloadTh);
    connect(&downloadTh,SIGNAL(started()),download,SLOT(onCreateTimer()));
    connect(&downloadTh,&QThread::finished,download,&QObject::deleteLater);
    downloadTh.start();
    //连接子线程传来的显示数据
    connect(download,SIGNAL(sendText(QString *,QString *)),this,SLOT(update(QString*,QString*)));//信号
    connect(this,SIGNAL(sendData2(int)),download,SLOT(timerStartOrStop(int)));





    customPlot = new QCustomPlot(this);
    setCentralWidget(customPlot);
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
    // now we assign some data, by accessing the QCPColorMapData instance of the color map: 现在我们通过访问颜色地图的QCPColorMapData实例来分配一些数据:

    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
        for (int yIndex=0; yIndex<ny; ++yIndex)
        {
            colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
            double r = 3*qSqrt(x*x+y*y)+1e-2;
            z=r;
            //qDebug()<<"x,y,z"<<x<<y<<z;
            //z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants) 偶极子辐射B场强(模物理常数)
            colorMap->data()->setCell(xIndex, yIndex, z);
        }
    }
    // add a color scale: 增设一个颜色刻度
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default) 刻度应该是带有刻度/轴标签的竖条(实际上atRight已经是默认值了)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale 将颜色映射与颜色比例关联起来
    colorScale->axis()->setLabel("Magnetic Field Strength");

    // set the color gradient of the color map to one of the presets: 将颜色映射的颜色渐变设置为预设之一
    colorMap->setGradient(QCPColorGradient::gpJet);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.
    // 我们也可以创建一个QCPColorGradient实例，并在渐变中添加自己的颜色，请参阅QCPColorGradient的文档。

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
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
}
//void MainWindow::update(QString time1,QString data1,QString time2,QString data2,QString time3,QString data3,QString time4,QString data4)
void MainWindow::update(QString *time,QString *data){
    double d[100];
    for(int i=0;i<sensornum;i++){
        d[i]=data[i].toDouble();
        qDebug()<<"d"<<d[i];
    }


    //控制最大为17，传入数据data默认在0-1之间。
    //qDebug()<<"x,y"<<x1<<y1<<x2<<y2<<x3<<y3<<x4<<y4<<d1<<d2<<d3<<d4;
    //double distance,min_distance[4]={100};
    //int min_sensor[4]={0};
    //bool first=true;
    // 创建 GSL 插值所需的变量
    //gsl_interp_accel *xacc = gsl_interp_accel_alloc();
    //gsl_interp_accel *yacc = gsl_interp_accel_alloc();
    //gsl_spline2d *spline = gsl_spline2d_alloc(gsl_interp2d_bicubic, sensornum, sensornum);
    // 初始化 x 和 y 方向上的插值
    //gsl_spline2d_init(spline, xg, yg, d, sensornum, sensornum);
    // for (int xIndex=0; xIndex<nx; ++xIndex)
    // {
    //     for (int yIndex=0; yIndex<ny; ++yIndex)
    //     {

    //         colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);

    //        // double interpolatedConcentration = gsl_spline2d_eval(spline, x, y, xacc, yacc);

    //         //double xInterpValue = gsl_interp_eval(xInterp, yg, xg, x, xacc);
    //         //double yInterpValue = gsl_interp_eval(yInterp, xg, yg, y, yacc);
    //         //double interpolatedConcentration = (xInterpValue + yInterpValue) / 2.0;

    //         // for(int i=0;i<4;i++){
    //         //     min_distance[i]=100;
    //         //     min_sensor[i]=0;
    //         // }
    //         // for(int i=0;i<sensornum;i++){
    //         //     distance=qSqrt((x-xg[i])*(x-xg[i])+(y-yg[i])*(y-yg[i]))+1e-2;
    //         //     for(int j=0;j<=i&&j<4;j++){
    //         //         if(min_distance[j]>distance){
    //         //             for(int k=2;k>=j;k--){
    //         //                 min_distance[k+1]=min_distance[k];
    //         //                 min_sensor[k+1]=min_sensor[k];
    //         //             }
    //         //             min_sensor[j]=i;
    //         //             min_distance[j]=distance;
    //         //             break;
    //         //         }
    //         //     }
    //         //     //if(first) qDebug()<<min_distance[0]<<min_distance[1]<<min_distance[2]<<min_distance[3];
    //         // }//计算距离最近的四个点
    //         // int mini;
    //         // qDebug()<<min_sensor[0];
    //         // for(int i=0;i<3;i++){
    //         //     for(int j=0;j<3-i;j++)
    //         //     if(xg[min_sensor[j+1]]<xg[min_sensor[j]]){
    //         //         mini=min_sensor[j];min_sensor[j]=min_sensor[j+1];min_sensor[j+1]=mini;
    //         //     }
    //         //     else if(yg[min_sensor[j+1]]<yg[min_sensor[j]]){
    //         //         mini=min_sensor[j];min_sensor[j]=min_sensor[j+1];min_sensor[j+1]=mini;
    //         //     }

    //         // }
    //         //first=false;
    //         //qDebug()<<min_distance[0];
    //         // x1=(int)x/2*2;x2=((int)x/2+1)*2;x3=(int)x/2*2;x4=((int)x/2+1)*2;
    //         // y1=(int)y/2;y2=(int)y/2;y3=((int)y+1)/2;y4=((int)y+1)/2;
    //         // x1=xg[min_sensor[0]];x2=xg[min_sensor[1]];x3=xg[min_sensor[2]];x4=xg[min_sensor[3]];
    //         // y1=yg[min_sensor[0]];y2=yg[min_sensor[1]];y3=yg[min_sensor[2]];y4=yg[min_sensor[3]];
    //         // d1=d[min_sensor[0]]*17;
    //         // d2=d[min_sensor[1]]*17;
    //         // d3=d[min_sensor[2]]*17;
    //         // d4=d[min_sensor[3]]*17;
    //         // double p=(y3-y2)*(x1-x2)+(x2-x3)*(y1-y2);
    //         //double u,v;
    //         // //r1= 3*qSqrt((x-x1)*(x-x1)+(y-y1)*(y-y1))+1e-2;
    //         // //r2 = 3*qSqrt((x-x2)*(x-x2)+(y-y2)*(y-y2))+1e-2;
    //         // u=((y3-y2)*(x-x2)+(x2-x3)*(y-y2))/p;
    //         // v=((y2-y1)*(x-x2)+(x1-x2)*(y-y2))/p;
    //         //z=d2*(1-u-v)+d1*u+d3*v;
    //         //u=(x-x1)/(x2-x1);
    //         //v=(y-y1)/(y3-y1);
    //         //z=d1*(1-u)*(1-v)+d2*(1-v)*u+d3*(1-u)*v+d4*u*v;
    //         //qDebug()<<"x,y"<<x<<y<<"      "<<x1<<y1<<x2<<y2<<x3<<y3<<x4<<y4<<d1<<d2<<d3<<u<<v<<z;
    //         //qDebug()<<"x,y,z"<<u<<v;
    //         //z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants) 偶极子辐射B场强(模物理常数)
    //         //colorMap->data()->setCell(xIndex, yIndex, interpolatedConcentration*17);



    //         colorMap->data()->setCell(xIndex, yIndex, z*17);
    //     }
    // }
    int p=qSqrt(sensornum);
    qDebug()<<p<<"p";
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4));
    for (int i=1;i<p;i++){
        for (int j=0;j<p-1;j++){
            x1=xg[(i-1)*p+j];x2=xg[(i-1)*p+j+1];x3=xg[i*p+j];x4=xg[i*p+j+1];
            y1=yg[(i-1)*p+j];y2=yg[(i-1)*p+j+1];y3=yg[i*p+j];y4=yg[i*p+j+1];
            d1=d[(i-1)*p+j]*17;
            d2=d[(i-1)*p+j+1]*17;
            d3=d[i*p+j]*17;
            d4=d[i*p+j+1]*17;
            qDebug()<<"x,y"<<x1<<y1<<x2<<y2<<x3<<y3<<x4<<y4<<d1<<d2<<d3<<d4;
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



    // for (int xIndex=0; xIndex<nx; ++xIndex)
    // {
    //     for (int yIndex=0; yIndex<ny; ++yIndex)
    //     {
    //         colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
    //         double u,v;
    //         //r1= 3*qSqrt((x-x1)*(x-x1)+(y-y1)*(y-y1))+1e-2;
    //         //r2 = 3*qSqrt((x-x2)*(x-x2)+(y-y2)*(y-y2))+1e-2;
    //         u=(y3-y2)*(x-x2)+(x2-x3)*(y-y2)/(y3-y2)*(x1-x2)+(x2-x3)*(y1-y2);
    //         v=(y2-y1)*(x-x2)+(x1-x2)*(y-y2)/(y3-y2)*(x1-x2)+(x2-x3)*(y1-y2);

    //         z=d1*(1-u)*(1-v)+d2*u*(1-v)+d3*u*v+d4*(1-u)*v;
    //         //qDebug()<<"x,y,z"<<u<<v;
    //         //z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants) 偶极子辐射B场强(模物理常数)
    //         colorMap->data()->setCell(xIndex, yIndex, z);
    //     }
    // }
    // add a color scale: 增设一个颜色刻度
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default) 刻度应该是带有刻度/轴标签的竖条(实际上atRight已经是默认值了)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale 将颜色映射与颜色比例关联起来
    colorScale->axis()->setLabel("Magnetic Field Strength");

    // set the color gradient of the color map to one of the presets: 将颜色映射的颜色渐变设置为预设之一
    colorMap->setGradient(QCPColorGradient::gpJet);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.
    // 我们也可以创建一个QCPColorGradient实例，并在渐变中添加自己的颜色，请参阅QCPColorGradient的文档。

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
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
}



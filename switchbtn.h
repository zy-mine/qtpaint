#ifndef SWITCHBTN_H
#define SWITCHBTN_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
//#include "mainwindow.h"

namespace Ui {
class SwitchBtn;
}

class SwitchBtn : public QWidget
{
    Q_OBJECT

public:
    explicit SwitchBtn(QWidget *parent = nullptr);
    ~SwitchBtn();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
signals:
    void btnChanged();
private:
    Ui::SwitchBtn *ui;
    bool isOff=true;
    QBrush offBgBrush= Qt::white;
    QBrush onBgBrush= Qt::cyan;

    QBrush offIndicatorBrush= Qt::cyan;
    QBrush onIndicatorBrush= Qt::white;

    QString offText="双线性插值";
    QString onText="双三次插值";

    QPropertyAnimation *animation=nullptr;
    int curX;

    //MainWindow *mainwindow;
};

#endif // SWITCHBTN_H

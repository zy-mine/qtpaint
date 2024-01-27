#include "switchbtn.h"
#include "ui_switchbtn.h"

SwitchBtn::SwitchBtn(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SwitchBtn)
{
    ui->setupUi(this);

    animation = new QPropertyAnimation(this);
    animation->setTargetObject(this);
    animation->setDuration(500);
    animation->setEasingCurve( QEasingCurve::InOutCubic);
    curX=height()/2;

    connect(animation, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value){
        curX=value.toInt();
        update();
    });
    //mainwindow=new MainWindow();
    //connect(this,SIGNAL(btnChanged()),mainwindow,SLOT(Slot1()));
}

SwitchBtn::~SwitchBtn()
{
    delete ui;
}
void SwitchBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(isOff ? offBgBrush:onBgBrush);
    painter.drawRoundedRect(this->rect(), height()/2, height()/2);

    painter.setBrush( isOff? offIndicatorBrush : onIndicatorBrush);
    painter.drawEllipse(QPoint(curX,height()/2), height()/2-8,height()/2-8);

    painter.setPen(Qt::black);
    painter.setFont(QFont("楷体",25));
    painter.drawText(this->rect(),(Qt::AlignCenter),isOff? offText:onText);
}

void SwitchBtn::mousePressEvent(QMouseEvent *event)
{
    if( event->button()==Qt::LeftButton ){
        isOff ?animation->setDirection(QVariantAnimation::Forward):animation->setDirection(QVariantAnimation::Backward);
        isOff=!isOff;
        animation->start();
        emit btnChanged();
    }
}

void SwitchBtn::resizeEvent(QResizeEvent *event)
{
    animation->setStartValue(height()/2);
    animation->setEndValue(width()-height()/2);
}

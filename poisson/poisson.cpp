#include "poisson.h"
#include "ui_poisson.h"

poisson::poisson(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::poisson)
{
    //初始化
    nface = 0;
    type = 0;
    Init();
    ui->setupUi(this);
}

void poisson::Init()
{
    n = 0;
    db = 0;
    minx = 9999; miny = 9999;
    maxx = -1; maxy = -1;
    //读取nface.jpg和(nface+1).jpg编号的两张图像
    char c[2];
    itoa(nface,c,10);
    name1 = ":imgs/";
    if (nface < 10)
        name1 = name1+c[0]+".jpg";
    else name1 = name1+c[0]+c[1]+".jpg";
    pix[0] = QImage(name1.c_str());
    for (int i = 0; i < 2; i++)
        ou[i] = QImage(name1.c_str());
    ou[2] = QImage(name2.c_str());

    itoa(nface+1,c,10);
    name2 = ":imgs/";
    if (nface < 10)
        name2 = name2+c[0]+".jpg";
    else name2 = name2+c[0]+c[1]+".jpg";
    pix[1] = QImage(name2.c_str());

    w0 = pix[0].width();
    h0 = pix[0].height();
    w1 = pix[1].width();
    h1 = pix[1].height();
    //图像位置
    pos[0].setX((700-w0-w1-35)/2+200);
    pos[0].setY(100);
    pos[1].setX(pos[0].x() + w0 + 35);
    pos[1].setY(pos[0].y());
    drag.setX(0);
    drag.setY(0);
    pre_pos.setX(0);
    pre_pos.setY(0);
    memset(bo,0,sizeof(bo));
    memset(in,0,sizeof(in));
    drag_b = false;
    hide = false;
}

int min(int a, int b)
{
    if (a<b) return a;
    return b;
}

int max(int a, int b)
{
    if (a>b) return a;
    return b;
}

void poisson::on_Reset_clicked()  //“重置”按钮触发槽函数
{
//    ui->a->setValue(1);   //各种参数都清零，并刷新画布
//    ui->b->setValue(5);
//    ui->p->setValue(0);
    Init(); //初始化
    update();
}

void poisson::on_Change_clicked() //更换图像按钮
{
    nface = (nface + 2) % 18;    //在多组图像中轮换
    Init(); //初始化所有参数
    update();
}


//左键单击
void poisson::mousePressEvent(QMouseEvent *m)
{
    if (!db && m->button() == Qt::LeftButton)
    {
        double x = m->x(), y = m->y();
        //画线
        if (n == 0 && x > pos[0].x() && x < pos[0].x()+w0
                && y > pos[0].y() && y < pos[0].y()+h0)
        {
            db = 1;
            n = 1;
            point[0] = m->pos();
            minx = min(point[n-1].x()-pos[0].x(),minx);
            miny = min(point[n-1].y()-pos[0].y(),miny);
            maxx = max(point[n-1].x()-pos[0].x(),maxx);
            maxy = max(point[n-1].y()-pos[0].y(),maxy);
        }
        //拖动
        else if (n > 0)
        {
            pre_pos.setX(x);
            pre_pos.setY(y);
            drag_b = true;
            hide = false;
        }
    }
}
//鼠标松开
void poisson::mouseReleaseEvent(QMouseEvent *m)
{
    setCursor(Qt::ArrowCursor);
    if (m->button() == Qt::LeftButton)
    {
        if (drag_b)
        {
            if (pos[2].x()>=pos[1].x() && pos[2].x()+ou[0].width()<=pos[1].x()+w1
                && pos[2].y()>=pos[1].y() && pos[2].y()+ou[0].height()<=pos[1].y()+h1)
                this->on_Run_clicked(false);
            drag_b = false;
        }
        db = 0;
    }
}

bool poisson::within(int y,int x)
{
    int count1 = 0, count2 = 0, count3 = 0, count4 = 0;
    for (int i = 1; i <= maxy - y; i++)
    {
        if (bo[y+i][x] && !bo[y+i-1][x]) count1++;
    }
    for (int i = 1; i <= y - miny; i++)
    {
        if (bo[y-i][x] && !bo[y-i+1][x]) count3++;
    }
    for (int j = 1; j <= maxx - x; j++)
    {
        if (bo[y][x+j] && !bo[y][x+j-1]) count2++;
    }
    for (int j = 1; j <= x - minx; j++)
    {
        if (bo[y][x-j] && !bo[y][x-j+1]) count4++;
    }
    if ( count1 == 0 || count2 == 0 || count3 == 0 || count4 == 0 ) return false;
    if ( (count1 % 2 == 1) || (count2 % 2 == 1)
         || (count3 % 2 == 1) || (count4 % 2 == 1) ) return true;
    return false;

}
//获取ROI图片
void poisson::get_ou()
{
    ou[0] = QImage(ouw,ouh,QImage::Format_RGBA8888);
    ou[1] = QImage(ouw,ouh,QImage::Format_RGBA8888);
    //ou[1] = ou[1].scaled(ouw,ouh);
    for (int i = miny; i <= maxy; i++)
    {
        for (int j = minx; j <= maxx; j++)
        if (within(i,j))
        //    if (bo[i][j])
        {
            in[i-miny][j-minx] = true;
            QRgb  rgb = pix[0].pixel(j,i);
            ou[0].setPixel(j-minx,i-miny,rgb);
            rgb = qRgb(255, 255, 255);
            ou[1].setPixel(j-minx,i-miny,rgb);
        }
        else
        {
            in[i-miny][j-minx] = false;
            QRgb rgb = QColor(0, 0, 0, 0).rgba();
            ou[0].setPixel(j-minx,i-miny,rgb);
            rgb = QColor(0, 0, 0, 0).rgb();
            ou[1].setPixel(j-minx,i-miny,rgb);
        }
    }

    //排除部分意外点
    for (int i = 0; i < ouh; i++)
        for (int j = 0; j < ouw; j++)
        if (in[i][j])
        {
            if ( (i-1>0 && i+1<ouh && !in[i-1][j] && !in[i+1][j])
               || (j-1>0 && j+1<ouw && !in[i][j-1] && !in[i][j+1]) )
            {
                in[i][j] = false;
                QRgb rgb = QColor(0, 0, 0, 0).rgba();
                ou[0].setPixel(j,i,rgb);
                rgb = QColor(0, 0, 0, 0).rgb();
                ou[1].setPixel(j,i,rgb);
            }
        }
    QImage temp0 = QImage(ouw,ouh,QImage::Format_RGBA8888);
    QImage temp1 = QImage(ouw,ouh,QImage::Format_RGBA8888);
    temp0 = ou[0].copy(0,0,ouw,ouh);
    temp1 = ou[1].copy(0,0,ouw,ouh);
    ou[0] = QImage(w0,h0,QImage::Format_RGBA8888);
    ou[1] = QImage(w0,h0,QImage::Format_RGBA8888);
    for (int i = 0; i < w0; i++)
        for (int j = 0; j < h0; j++)
        if (i >= minx && i <= maxx &&
            j >= miny && j <= maxy && in[j-miny][i-minx])
        {
            ou[0].setPixel(i,j,temp0.pixel(i-minx,j-miny));
            ou[1].setPixel(i,j,temp1.pixel(i-minx,j-miny));
        }
        else
        {
            ou[0].setPixel(i,j,QColor(0, 0, 0, 0).rgba());
            ou[1].setPixel(i,j,QColor(0, 0, 0, 0).rgb());
        }
    ou[0].save("imgs/ou0.png");
    ou[0].save("imgs/ou0.jpg");
    ou[1].save("imgs/ou1.jpg");
    ou[0] = QImage(ouw,ouh,QImage::Format_RGBA8888);
    ou[0] = temp0.copy(0,0,ouw,ouh);
    update();
}

//鼠标移动
void poisson::mouseMoveEvent(QMouseEvent *m)  //实时追踪鼠标运动
{
    if (db && n < 4000)
    {
        double x = m->x(), y = m->y();
        if (x > pos[0].x() && x < pos[0].x()+w0
            && y > pos[0].y() && y < pos[0].y()+h0)
        {
            //若鼠标运动到距离起始点半径5个坐标单位内，则鼠标光标变为十字形
            if ( (n>10) && (abs(x-point[0].x())<=5) &&
                 (abs(y-point[0].y())<=5) )
            {

                setCursor(Qt::CrossCursor);
                point[n++] = point[0];
                db = 0;
                ouw = maxx-minx+1;
                ouh = maxy-miny+1;
                for (int i = 1; i < n; i++)
                {
                    int xx = point[i].x()-pos[0].x();
                    int yy = point[i].y()-pos[0].y();
                    int xx2 = point[i-1].x()-pos[0].x();
                    int yy2 = point[i-1].y()-pos[0].y();
                    bo[yy][xx] = true;
                    if (xx > xx2)
                        for (int j = 1; j < xx-xx2; j++) bo[yy][xx-j] = true;
                    else
                        for (int j = 1; j < xx2-xx; j++) bo[yy][xx+j] = true;
                    if (yy > yy2)
                        for (int j = 1; j < yy-yy2; j++) bo[yy-j][xx2] = true;
                    else
                        for (int j = 1; j < yy2-yy; j++) bo[yy+j][xx2] = true;

                }
                //记录初始位置
                pos[2].setX(minx+pos[0].x());
                pos[2].setY(miny+pos[0].y());
                //获取ROI图片
                get_ou();
            }
            else
            {
                setCursor(Qt::ArrowCursor);
                point[n++] = m->pos();
            }
            minx = min(point[n-1].x()-pos[0].x(),minx);
            miny = min(point[n-1].y()-pos[0].y(),miny);
            maxx = max(point[n-1].x()-pos[0].x(),maxx);
            maxy = max(point[n-1].y()-pos[0].y(),maxy);
            repaint();
        }
    }
    //拖动
    else if (!db && drag_b && n > 0)
    {
        drag.setX(m->x()-pre_pos.x());
        drag.setY(m->y()-pre_pos.y());
        pos[2].setX( pos[2].x()+drag.x() );
        pos[2].setY( pos[2].y()+drag.y() );
        pre_pos.setX(m->x());
        pre_pos.setY(m->y());
        repaint();
    }
}

void poisson::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    //绘制测试图像
    for (int i = 0; i < 2; i++)
        painter.drawImage(pos[i].x(),pos[i].y(),pix[i]);
    //for (int i = 0; i < 2; i++) painter.drawImage(pos[0].x(),pos[0].y()*(i+2),ou[i]);
    //绘制截图
    if (n > 0 && !db && !hide)
    {
        painter.drawImage(pos[2].x(),pos[2].y(),ou[0]);
    }

    painter.setPen(QPen(Qt::black,4.0));
    pen.setColor(qRgb(255, 255, 255));
    pen.setWidthF(2);
    painter.setPen(pen);    //设置画笔
    //绘制轮廓
    if (pre_pos.x() == 0)
        for (int i = 1; i < n; i++)
            painter.drawLine(point[i],point[i-1]);

    //绘制工具条
    if (type == 3)
    {
        ui->Low->show();
        ui->Low_v->show();
        ui->Low_t->show();
        ui->High->show();
        ui->High_v->show();
        ui->High_t->show();
        ui->Kernel->show();
        ui->Kernel_v->show();
        ui->Kernel_t->show();

        ui->Alpha->hide();
        ui->Alpha_v->hide();
        ui->Alpha_t->hide();
        ui->Beta->hide();
        ui->Beta_v->hide();
        ui->Beta_t->hide();

        ui->Red->hide();
        ui->Red_v->hide();
        ui->Red_t->hide();
        ui->Green->hide();
        ui->Green_v->hide();
        ui->Green_t->hide();
        ui->Blue->hide();
        ui->Blue_v->hide();
        ui->Blue_t->hide();
    }
    else if (type == 4)
    {
        ui->Alpha->show();
        ui->Alpha_v->show();
        ui->Alpha_t->show();
        ui->Beta->show();
        ui->Beta_v->show();
        ui->Beta_t->show();

        ui->Low->hide();
        ui->Low_v->hide();
        ui->Low_t->hide();
        ui->High->hide();
        ui->High_v->hide();
        ui->High_t->hide();
        ui->Kernel->hide();
        ui->Kernel_v->hide();
        ui->Kernel_t->hide();

        ui->Red->hide();
        ui->Red_v->hide();
        ui->Red_t->hide();
        ui->Green->hide();
        ui->Green_v->hide();
        ui->Green_t->hide();
        ui->Blue->hide();
        ui->Blue_v->hide();
        ui->Blue_t->hide();
    }
    else if (type == 5)
    {
        ui->Red->show();
        ui->Red_v->show();
        ui->Red_t->show();
        ui->Green->show();
        ui->Green_v->show();
        ui->Green_t->show();
        ui->Blue->show();
        ui->Blue_v->show();
        ui->Blue_t->show();

        ui->Low->hide();
        ui->Low_v->hide();
        ui->Low_t->hide();
        ui->High->hide();
        ui->High_v->hide();
        ui->High_t->hide();
        ui->Kernel->hide();
        ui->Kernel_v->hide();
        ui->Kernel_t->hide();

        ui->Alpha->hide();
        ui->Alpha_v->hide();
        ui->Alpha_t->hide();
        ui->Beta->hide();
        ui->Beta_v->hide();
        ui->Beta_t->hide();
    }
    else
    {
        ui->Alpha->hide();
        ui->Alpha_v->hide();
        ui->Alpha_t->hide();
        ui->Beta->hide();
        ui->Beta_v->hide();
        ui->Beta_t->hide();

        ui->Low->hide();
        ui->Low_v->hide();
        ui->Low_t->hide();
        ui->High->hide();
        ui->High_v->hide();
        ui->High_t->hide();
        ui->Kernel->hide();
        ui->Kernel_v->hide();
        ui->Kernel_t->hide();

        ui->Red->hide();
        ui->Red_v->hide();
        ui->Red_t->hide();
        ui->Green->hide();
        ui->Green_v->hide();
        ui->Green_t->hide();
        ui->Blue->hide();
        ui->Blue_v->hide();
        ui->Blue_t->hide();

    }
}


poisson::~poisson()
{
    delete ui;
}

void poisson::on_comboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "Normal Clone") type = 0;
    else if (arg1 == "Mixed Clone") type = 1;
    else if (arg1 == "Feature Exc") type = 2;
    else if (arg1 == "Texture Flat") type = 3;
    else if (arg1 == "Illum Change") type = 4;
    else if (arg1 == "Color Change") type = 5;
    char c[2];
    itoa(nface,c,10);
    name1 = ":imgs/";
    if (nface < 10)
        name1 = name1+c[0]+".jpg";
    else name1 = name1+c[0]+c[1]+".jpg";

    itoa(nface+1,c,10);
    name2 = ":imgs/";
    if (nface < 10)
        name2 = name2+c[0]+".jpg";
    else name2 = name2+c[0]+c[1]+".jpg";
    update();
}

void poisson::on_Low_sliderMoved(int position)
{
    if (position >= ui->High->value())
        ui->High->setValue(position+1);
    QString s = QString::number(position, 10);
    ui->Low_v->setText(s);
    s = QString::number(ui->High->value(), 10);
    ui->High_v->setText(s);
}

void poisson::on_High_sliderMoved(int position)
{
    if (position <= ui->Low->value())
        ui->Low->setValue(position-1);
    QString s = QString::number(position, 10);
    ui->High_v->setText(s);
    s = QString::number(ui->Low->value(), 10);
    ui->Low_v->setText(s);
}

void poisson::on_Kernel_sliderMoved(int position)
{
    QString s = QString::number(position*2+1, 10);
    ui->Kernel_v->setText(s);
}

void poisson::on_Alpha_sliderMoved(int position)
{
    double p = position;
    p = p / 100.0;
    QString s = QString::number(p, 'g', 2);
    ui->Alpha_v->setText(s);
}

void poisson::on_Beta_sliderMoved(int position)
{
    double p = position;
    p = p / 100.0;
    QString s = QString::number(p, 'g', 2);
    ui->Beta_v->setText(s);
}

void poisson::on_Red_sliderMoved(int position)
{
    QString s = QString::number(position, 10);
    ui->Red_v->setText(s);
}

void poisson::on_Green_sliderMoved(int position)
{
    QString s = QString::number(position, 10);
    ui->Green_v->setText(s);
}

void poisson::on_Blue_sliderMoved(int position)
{
    QString s = QString::number(position, 10);
    ui->Blue_v->setText(s);
}

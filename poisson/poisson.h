#ifndef POISSON_H
#define POISSON_H

#include <QMainWindow>
#include <QtGui>
#include "stdio.h"
#include "stdlib.h"
#include "iostream"
#include <QPainter>
#include <QPointF>
#include <QLineF>
#include <QPolygon>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <complex>
#include "math.h"

using namespace std;
using namespace cv;

namespace Ui {
class poisson;
}

class poisson : public QMainWindow
{
    Q_OBJECT

public:
    explicit poisson(QWidget *parent = 0);
    ~poisson();
    //点个数、显示图像个数、切换序号
    int n,k,nface;
    int minx,miny,maxx,maxy;
    int ouw,ouh;
    string name1, name2;
    QPoint point[4000]; //点坐标
    QPoint pos[3];   //图片位置
    QPoint drag, pre_pos;    //拖动
    int db; //动态flag
    QImage pix[2];  //存储图像
    QImage ou[5];  //存储ROI
    double w0,h0,w1,h1; //图像宽高
    void Init();    //初始化函数
    void get_ou();
    bool within(int i,int j);
    bool bo[600][600];
    bool in[600][600];
    bool drag_b;
    bool hide;  //隐藏截图
    int type;
    Mat src_ou, dst_fstar;
    //梯度
    Mat Gr_dst_x, Gr_dst_y, Gr_src_x, Gr_src_y;
    Mat Gr_src_ou_x, Gr_src_ou_y, Gr_dst_fstar_x, Gr_dst_fstar_y;
    //卷积核
    Mat kernel_x, kernel_y;
    //腐蚀核
    Mat erode_k;
    void normal_clone(Mat &dst);
    void mixed_clone(Mat &dst);
    void get_mix(Mat dst);
    void feature_exc(Mat &src, Mat &dst);
    void feature_exc_run(Mat &dst, Mat &dst_color, Mat &dst_grey, int flag);
    void texture_flat(Mat src);
    void illum_change(Mat src);
    void color_change(Mat src);
    void get_gr(Mat &img, Mat &ou_color);
    void init(Mat &img, Mat &ou_color, Mat &ou_grey);
    void scalar_product(Mat mat, float r, float g, float b);
    void mul(Mat mat1, Mat mat2, Mat mat3);
    void run(Mat &img, Mat &ou_grey, Mat &result);
    void dst(double *diff, double *st, int h, int w);
    void transpose(double *mat, double *mat_t, int h, int w);
    void poisson_solver(const Mat &img, Mat &lap, Mat &result);

private slots:  //设置槽函数
    void on_Run_clicked(bool checked);   //点击“Run”按键的关联函数

    void on_Reset_clicked(); //点击“Reset”按键的关联函数

    void on_Change_clicked();   //更换图像

    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_Low_sliderMoved(int position);

    void on_High_sliderMoved(int position);

    void on_Kernel_sliderMoved(int position);

    void on_Alpha_sliderMoved(int position);

    void on_Beta_sliderMoved(int position);

    void on_Red_sliderMoved(int position);

    void on_Green_sliderMoved(int position);

    void on_Blue_sliderMoved(int position);

private:
    Ui::poisson *ui;
    void mouseMoveEvent(QMouseEvent *m);    //鼠标追踪函数
    void mousePressEvent(QMouseEvent *m);   //捕捉鼠标点击画布的信息
    void mouseReleaseEvent(QMouseEvent *m); //鼠标释放
    void paintEvent(QPaintEvent*);  //绘制
};

#endif // POISSON_H

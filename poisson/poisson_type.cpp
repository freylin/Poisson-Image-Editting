#include "poisson.h"
#include "ui_poisson.h"

void poisson::on_Run_clicked(bool checked)
{
    if (!checked)
    {
        pix[0] = QImage(name1.c_str());
        pix[1] = QImage(name2.c_str());
        Mat src,dst;
        if (nface<10)
        {
            src = imread(name1.substr(1,10));
            dst = imread(name2.substr(1,10));
        }
        else
        {
            src  = imread(name1.substr(1,11));
            dst = imread(name2.substr(1,11));
        }

        if (type<3)
        {
            ou[2] = QImage(w1,h1,QImage::Format_RGBA8888);
            ou[3] = QImage(w1,h1,QImage::Format_RGBA8888);
            for (int i = 0; i < w1; i++)
                for (int j = 0; j < h1; j++)
                if (i >= pos[2].x()-pos[1].x() && i <= pos[2].x()-pos[1].x()+maxx-minx &&
                    j >= pos[2].y()-pos[1].y() && j <= pos[2].y()-pos[1].y()+maxy-miny &&
                    in[j-(pos[2].y()-pos[1].y())][i-(pos[2].x()-pos[1].x())])
                {
                    ou[2].setPixel(i,j,QColor(255, 255, 255, 0).rgb());
                    ou[3].setPixel(i,j,pix[0].pixel(i-(pos[2].x()-pos[1].x())+minx,j-(pos[2].y()-pos[1].y())+miny));
                }
                else
                {
                    ou[2].setPixel(i,j,QColor(0, 0, 0, 0).rgb());
                    ou[3].setPixel(i,j,QColor(0, 0, 0, 0).rgb());
                }
            ou[2].save("imgs/ou2.jpg");
            ou[3].save("imgs/ou3.jpg");

            if (type == 0) normal_clone(dst);
            else if (type == 1) mixed_clone(dst);
            else
            {
                feature_exc(src, dst);
                pix[0] =QImage("imgs/out1.jpg");
            }
        }
        else if (type == 3)
        {
            texture_flat(src);
        }
        else if (type == 4)
        {
            illum_change(src);
        }
        else if (type == 5)
        {
            color_change(src);
        }
        pix[1] = QImage("imgs/out.jpg");
        hide = true;
        waitKey(0);

        update();
    }
}

// 普通poisson clone
void poisson::normal_clone(Mat &dst)
{
    IplImage qImg;
    Mat blend;
    blend.create(dst.size(), CV_8UC3);

    Mat dst_color = imread("imgs/ou3.jpg");
    Mat dst_grey = imread("imgs/ou2.jpg");
    cvtColor(dst_grey,dst_grey,CV_RGB2GRAY);
    dst_grey.convertTo(dst_grey, CV_8UC1);

    init(dst, dst_color, dst_grey);
    mul(Gr_src_ou_x, Gr_src_x, src_ou);
    mul(Gr_src_ou_y, Gr_src_y, src_ou);
    run(dst, dst_grey, blend);

    qImg = IplImage(blend);
    cvSaveImage("imgs/out.jpg", &qImg);
}

void poisson::get_mix(Mat dst)
{
    float x1,x2,y1,y2;
    for (int i = 0; i < dst.size().height; i++)
    {
        for (int j = 0; j < dst.size().width; j++)
        {
            int jd = j*dst.channels();
            for (int k = jd; k < jd+dst.channels(); k++)
            {
                x1 = Gr_src_x.at<float>(i, k);
                y1 = Gr_src_y.at<float>(i, k);
                x2 = Gr_dst_x.at<float>(i, k);
                y2 = Gr_dst_y.at<float>(i, k);
                if (abs(x1 - y1) >= abs(x2 - y2))
                {
                    Gr_src_ou_x.at<float>(i, k) = x1 * src_ou.at<float>(i, j);
                    Gr_src_ou_y.at<float>(i, k) = y1 * src_ou.at<float>(i, j);
                }
                else
                {
                    Gr_src_ou_x.at<float>(i, k) = x2 * src_ou.at<float>(i, j);
                    Gr_src_ou_y.at<float>(i, k) = y2 * src_ou.at<float>(i, j);
                }
            }
        }
    }
}

// 混合poisson clone
void poisson::mixed_clone(Mat &dst)
{
    IplImage qImg;
    Mat blend;
    blend.create(dst.size(), CV_8UC3);

    Mat dst_color = imread("imgs/ou3.jpg");
    Mat dst_grey = imread("imgs/ou2.jpg");
    cvtColor(dst_grey,dst_grey,CV_RGB2GRAY);
    dst_grey.convertTo(dst_grey, CV_8UC1);

    init(dst, dst_color, dst_grey);
    get_mix(dst);
    run(dst, dst_grey, blend);

    qImg = IplImage(blend);
    cvSaveImage("imgs/out.jpg", &qImg);
}

void poisson::feature_exc_run(Mat &dst, Mat &dst_color, Mat &dst_grey, int flag)
{
    IplImage qImg;
    Mat blend;
    blend.create(dst.size(), CV_8UC3);

    init(dst, dst_color, dst_grey);

    Mat grey = Mat(dst_color.size(), CV_8UC1);
    cvtColor(dst_color, grey, COLOR_BGR2GRAY);
    vector <Mat> temp;
    Mat grey2 = Mat(dst_color.size(), CV_8UC3);
    split(grey2, temp);
    for (int i = 0; i < 3; i++)
        grey.copyTo(temp[i]);
    merge(temp, grey2);
    //求梯度
    filter2D(grey2, Gr_src_x, CV_32F, kernel_x);
    filter2D(grey2, Gr_src_y, CV_32F, kernel_y);

    mul(Gr_src_ou_x, Gr_src_x, src_ou);
    mul(Gr_src_ou_y, Gr_src_y, src_ou);

    run(dst, dst_grey, blend);
    qImg = IplImage(blend);
    if (flag == 1) cvSaveImage("imgs/out.jpg", &qImg);
    else cvSaveImage("imgs/out1.jpg", &qImg);
}

// 纹理互换
void poisson::feature_exc(Mat &src, Mat &dst)
{
    Mat dst_color = imread("imgs/ou3.jpg");
    Mat dst_grey = imread("imgs/ou2.jpg");
    cvtColor(dst_grey,dst_grey,CV_RGB2GRAY);
    dst_grey.convertTo(dst_grey, CV_8UC1);

    feature_exc_run(dst, dst_color, dst_grey, 1);

    ou[4] = QImage(w0,h0,QImage::Format_RGBA8888);
    for (int i = 0; i < w0; i++)
        for (int j = 0; j < h0; j++)
        if (i >= minx && i <= maxx &&
            j >= miny && j <= maxy &&
            in[j-miny][i-minx])
        {
            ou[4].setPixel(i,j,pix[1].pixel(i-minx+(pos[2].x()-pos[1].x()),j-miny+(pos[2].y()-pos[1].y())));
        }
        else
        {
            ou[4].setPixel(i,j,QColor(0, 0, 0, 0).rgb());
        }
    ou[4].save("imgs/ou4.jpg");

    dst_color = imread("imgs/ou4.jpg");
    dst_grey = imread("imgs/ou1.jpg");
    cvtColor(dst_grey, dst_grey, COLOR_BGR2GRAY);
    dst_grey.convertTo(dst_grey, CV_8UC1);

    feature_exc_run(src, dst_color, dst_grey, 2);
}


void poisson::texture_flat(Mat src)
{
    Mat ou = imread("imgs/ou1.jpg");
    double low;
    double high;
    int kernel;
    low = ui->Low->value();
    high = ui->High->value();
    kernel = ui->Kernel->value()*2+1;
    Mat result;
    result.create(src.size(), src.type());

    Mat ou_gray = Mat::zeros(ou.size(), CV_8UC1);

    if (ou.channels() == 3)
        cvtColor(ou, ou_gray, COLOR_BGR2GRAY);
    else
        ou_gray = ou;

    Mat ou0 = imread("imgs/ou0.jpg");
    ou0.convertTo(ou0, CV_8UC3);

    init(src, ou0, ou_gray);

    Mat out = imread("imgs/ou0.jpg");
    out.convertTo(out, CV_8UC1);
    //canny
    Canny(ou0, out, low, high, kernel);
    Size size = Gr_src_x.size();
    Mat black(size, CV_32FC3);
    Mat bw(size, CV_8UC1);
    for (int i = 0; i < size.height; i++)
        for (int j = 0; j < size.width; j++)
        {
            black.at<Vec3i>(i, j) = 0;
            int t = (int)out.at<uchar>(i, j);
            if (t == 255)
                bw.at<uchar>(i, j) = 0;
            else bw.at<uchar>(i, j) = 255;
        }

    black.copyTo(Gr_src_x, bw);
    black.copyTo(Gr_src_y, bw);
    mul(Gr_src_ou_x, Gr_src_x, src_ou);
    mul(Gr_src_ou_y, Gr_src_y, src_ou);

    run(src, ou_gray, result);

    IplImage qImg;
    qImg = IplImage(result);
    cvSaveImage("imgs/out.jpg", &qImg);
    //imshow("res",dst);
}

void poisson::illum_change(Mat src)
{
    Mat ou = imread("imgs/ou1.jpg");
    Mat result;
    result.create(src.size(), src.type());

    float alpha;
    float beta;
    alpha = ui->Alpha->value()/100.0;
    beta = ui->Beta->value()/100.0;
    Mat ou_gray = Mat::zeros(ou.size(), CV_8UC1);

    if (ou.channels() == 3)
        cvtColor(ou, ou_gray, COLOR_BGR2GRAY);
    else
        ou_gray = ou;

    Mat ou0 = imread("imgs/ou0.jpg");
    ou0.convertTo(ou0, CV_8UC3);

    init(src, ou0, ou_gray);

    mul(Gr_src_ou_x, Gr_src_x, src_ou);
    mul(Gr_src_ou_y, Gr_src_y, src_ou);

    Mat mag = Mat(src.size(), CV_32FC3);
    magnitude(Gr_src_ou_x, Gr_src_ou_y, mag);
    float minf = 0.00000001;

    for (int i = 0; i < mag.cols; i++)
        for (int j = 0; j < mag.rows; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                if (mag.at<cv::Vec3f>(j, i)[k] <= 0)
                    mag.at<cv::Vec3f>(j, i)[k] = minf;
            }
        }

    Mat multX, multY, multx_temp, multy_temp;
    double ab;
    ab = pow(alpha, beta);
    multiply(Gr_src_ou_x, ab, multX);
    pow(mag, -1 * beta, multx_temp);
    multiply(multX, multx_temp, Gr_src_ou_x);

    multiply(Gr_src_ou_y, ab, multY);
    pow(mag, -1 * beta, multy_temp);
    multiply(multY, multy_temp, Gr_src_ou_y);

    Size size = Gr_src_ou_x.size();
    Mat bw(size, CV_8UC1);

    for (int i = 0; i < size.height; i++)
        for (int j = 0; j < size.width; j++)
        {
            int t = (int)Gr_src_ou_x.at<uchar>(i, j);
            if (t == 255)
                bw.at<uchar>(i, j) = 0;
            else bw.at<uchar>(i, j) = 255;
        }

    Gr_src_ou_x.copyTo(Gr_src_ou_x, bw);
    Gr_src_ou_y.copyTo(Gr_src_ou_y, bw);

    run(src, ou_gray, result);

    IplImage qImg;
    qImg = IplImage(result);
    cvSaveImage("imgs/out.jpg", &qImg);
    //imshow("res",dst);
}

void poisson::color_change(Mat src)
{
    Mat ou = imread("imgs/ou1.jpg");
    Mat result;
    result.create(src.size(), src.type());
    Mat ou_gray = Mat::zeros(ou.size(), CV_8UC1);
    float r, g, b;
    r = ui->Red->value();
    r = r / 255.0f;
    g = ui->Green->value();
    g = g / 255.0f;
    b = ui->Blue->value();
    b = b / 255.0f;
    if (ou.channels() == 3)
        cvtColor(ou, ou_gray, COLOR_BGR2GRAY);
    else
        ou_gray = ou;

    Mat ou0 = imread("imgs/ou0.jpg");
    ou0.convertTo(ou0, CV_8UC3);

    init(src, ou0, ou_gray);

    mul(Gr_src_ou_x, Gr_src_x, src_ou);
    mul(Gr_src_ou_y, Gr_src_y, src_ou);

    vector <Mat> mat2;
    split(Gr_src_ou_x, mat2);
    multiply(mat2[0], b, mat2[0]);
    multiply(mat2[1], g, mat2[1]);
    multiply(mat2[2], r, mat2[2]);
    merge(mat2, Gr_src_ou_x);

    split(Gr_src_ou_y, mat2);
    multiply(mat2[0], b, mat2[0]);
    multiply(mat2[1], g, mat2[1]);
    multiply(mat2[2], r, mat2[2]);
    merge(mat2, Gr_src_ou_y);

    run(src, ou_gray, result);

    IplImage qImg;
    qImg = IplImage(result);
    cvSaveImage("imgs/out.jpg", &qImg);
    //imshow("res",dst);
}

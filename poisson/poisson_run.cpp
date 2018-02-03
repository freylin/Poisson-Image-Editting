#include "poisson.h"

void poisson::mul(Mat mat1, Mat mat2, Mat mat3)
{
    vector <Mat> mat4;
    vector <Mat> mat5;
    split(mat1, mat4);
    split(mat2, mat5);
    for (int i = 0; i < 3; i++)
        multiply(mat5[i], mat3, mat4[i]);
    merge(mat4, mat1);
}

void poisson::run(Mat &img, Mat &ou_grey, Mat &result)
{
    double alpha = 1.0;
    double beta = 0.0;
    alpha = alpha / 255.0;
    //取反操作
    bitwise_not(ou_grey, ou_grey);

    ou_grey.convertTo(dst_fstar, CV_32FC1, alpha, beta);
    img.convertTo(Gr_dst_fstar_x, CV_32FC3, alpha, beta);
    mul(Gr_dst_fstar_x, Gr_dst_x, dst_fstar);
    img.convertTo(Gr_dst_fstar_y, CV_32FC3, alpha, beta);
    mul(Gr_dst_fstar_y, Gr_dst_y, dst_fstar);

    Mat lap_x = Mat(img.size(), CV_32FC3);
    Mat lap_y = Mat(img.size(), CV_32FC3);
    Mat lap = Mat(img.size(), CV_32FC1);
    //对梯度再卷积，求Laplacian
    filter2D(Gr_dst_fstar_x + Gr_src_ou_x, lap_x, CV_32F, kernel_x);
    filter2D(Gr_dst_fstar_y + Gr_src_ou_y, lap_y, CV_32F, kernel_y);
    //分通道处理
    vector <Mat> rgb_x, rgb_y, s_rgb, rgb;
    split(lap_x, rgb_x);
    split(lap_y, rgb_y);
    split(img, s_rgb);
    split(img, rgb);
    for (int i = 0; i < 3; i++)
    {
        lap = rgb_x[i]+rgb_y[i];
        poisson_solver(s_rgb[i], lap, rgb[i]);
    }
    merge(rgb, result);
}

//get合成
void poisson::poisson_solver(const Mat &img, Mat &lap, Mat &result)
{
    int w = img.size().width;
    int h = img.size().height;
    int size = w * h;
    int size2 = (w - 2) * (h - 2);
    double *bound = new double[size];
    double *img_diff = new double[size];
    double *st = new double[size2];
    double *st_t = new double[size2];
    double *ist = new double[size2];
    double *ist_t = new double[size2];

    //保持边界
    Mat img_bound = img.clone();
    for (int i = 1; i < h - 1; i++)
        for (int j = 1; j < w - 1; j++)
        {
            img_bound.at<uchar>(i, j) = 0;
        }

    for (int i = 1; i < h - 1; i++)
    {
        int r = w * i;
        for (int j = 1; j < w - 1; j++)
        {
            bound[r + j] = (int)img_bound.at<uchar>(i, j - 1)
                                + (int)img_bound.at<uchar>(i, j + 1)
                                + (int)img_bound.at<uchar>(i - 1, j)
                                + (int)img_bound.at<uchar>(i + 1, j)
                                -4 * (int)img_bound.at<uchar>(i, j);
        }
    }

    //少边界
    for (int i = 1; i < h - 1; i++)
    {
        int r = w * i;
        int r2 = (w - 2) * (i - 1);
        for (int j = 1; j < w - 1; j++)
        {
            img_diff[r2 + j - 1] = (float)(lap.at<float>(i, j) - bound[r + j]);
        }
    }

    //DST
    dst(img_diff, st, h - 2, w - 2);
    transpose(st, st_t, h - 2, w - 2);

    dst(st_t, st, w - 2, h - 2);
    transpose(st, st_t, w - 2, h - 2);

    for (int i = 0; i < w - 2; i++)
    {
        float cosin = (float)2.0f * cos(CV_PI*(i + 1) / ((double)(w - 1)));
        for (int j = 0; j < h - 2; j++)
        {
            int r = j * (w - 2) + i;
            st_t[r] = st_t[r] / (cosin + 2 * cos(CV_PI*(j + 1) / ((double)(h - 1))) - 4);
        }
    }

    //IDST
    dst(st_t, ist, h - 2, w - 2);
    for (int i = 0; i < h - 2; i++)
    {
        int r = (w - 2) * i;
        for (int j = 0; j < w - 2; j++)
        {
            ist[r + j] = (double)(2 * ist[r + j]) / (h - 1);
        }
    }
    transpose(ist, ist_t, h - 2, w - 2);

    dst(ist_t, ist, w - 2, h - 2);
    for (int i = 0; i < w - 2; i++)
    {
        int r = (h - 2) * i;
        for (int j = 0; j < h - 2; j++)
        {
            ist[r + j] = (double)(2 * ist[r + j]) / (w - 1);
        }
    }
    transpose(ist, ist_t, w - 2, h - 2);

    for (int i = 0; i < h; i++)
    {
        int r = (i - 1)*(w - 2);
        for (int j = 0; j < w; j++)
        {
            double t;
            if (i > 0 && i < h - 1 && j > 0 && j < w - 1)
                t = ist_t[r + (j - 1)];
            else
                t = (double)img.at<uchar>(i, j);

            if (t < 0.0)
                result.at<uchar>(i, j) = 0;
            else if (t > 255.0)
                result.at<uchar>(i, j) = 255;
            else
                result.at<uchar>(i, j) = (uchar)t;
        }
    }
}

void poisson::get_gr(Mat &img, Mat &ou_color)
{
    Size img_s = img.size();
    Gr_dst_x = Mat(img_s, CV_32FC3);
    Gr_dst_y = Mat(img_s, CV_32FC3);
    filter2D(img, Gr_dst_x, CV_32F, kernel_x);
    filter2D(img, Gr_dst_y, CV_32F, kernel_y);

    Gr_src_x = Mat(img_s, CV_32FC3);
    Gr_src_y = Mat(img_s, CV_32FC3);
    filter2D(ou_color, Gr_src_x, CV_32F, kernel_x);
    filter2D(ou_color, Gr_src_y, CV_32F, kernel_y);
}

void poisson::init(Mat &img, Mat &ou_color, Mat &ou_grey)
{
    double alpha = 1.0;
    double beta = 0.0;
    alpha = alpha / 255.0;
    Size img_s = img.size();
    kernel_x = Mat::zeros(1, 3, CV_8S);
    kernel_y = Mat::zeros(3, 1, CV_8S);
    for (int i = 0; i < 3; i++)
    {
        kernel_x.at<char>(0, i) = -i+(i/2*3);
        kernel_y.at<char>(i, 0) = -i+(i/2*3);
    }
    Gr_dst_fstar_x = Mat(img_s, CV_32FC3);
    Gr_dst_fstar_y = Mat(img_s, CV_32FC3);
    dst_fstar = Mat(ou_grey.size(), CV_32FC1);

    //腐蚀，让边界更平滑
    erode_k = Mat(Size(3, 3), CV_8UC1);
    erode_k.setTo(Scalar(1));
    erode(ou_grey, ou_grey, erode_k, Point(-1, -1), 3);

    src_ou = Mat(ou_grey.size(), CV_32FC1);
    ou_grey.convertTo(src_ou, CV_32FC1, alpha, beta);

    Gr_src_ou_x = Mat(img_s, CV_32FC3);
    Gr_src_ou_y = Mat(img_s, CV_32FC3);
    img.convertTo(Gr_src_ou_x, CV_32FC3, alpha, beta);
    img.convertTo(Gr_src_ou_y, CV_32FC3, alpha, beta);

    //求梯度
    get_gr(img, ou_color);
}

#include "poisson.h"

//离散正弦变换
void poisson::dst(double *diff, double *st, int h, int w)
{
    int h2 = h * 2;
    Mat temp = Mat(h2 + 2, 1, CV_32F);
    for (int i = 0; i < w; i++)
    {
        for (int j = -1; j <= h2; j++)
        {
            if (j == -1 || j == h)
                temp.at<float>(j + 1, 0) = 0.0;
            else if (j < h)
                temp.at<float>(j + 1, 0) = (float)diff[j * w + i];
            else
                temp.at<float>(j + 1, 0) = -(float)diff[(- j + h2) * w + i];
        }

        Mat result;
        Size s = temp.size();
        Mat result2[] = { Mat_<float>(temp), Mat::zeros(s, CV_32F) };
        merge(result2, 2, result);

        dft(result, result, 0, 0);
        s = result.size();
        Mat result3[] = { Mat::zeros(s, CV_32F), Mat::zeros(s, CV_32F) };
        split(result, result3);

        double d = -2.0 * imag(sqrt(complex<double>(-1)));
        for (int j = 1; j < h + 1; j++)
        {
            int r = w * (j - 1);
            st[r + i] = (float)(result3[1].at<float>(j, 0) / d);
        }
    }
}

void poisson::transpose(double *mat, double *mat_t, int h, int w)
{
    Mat mat2 = Mat(h, w, CV_32FC1);
    for (int i = 0; i < h; i++)
    {
        int r = w * i;
        for (int j = 0; j < w; j++)
        {
            mat2.at<float>(i, j) = (float)mat[r + j];
        }
    }
    int temp;
    temp = h;
    h = w;
    w = temp;
    Mat trans = Mat(h, w, CV_32FC1);
    trans = mat2.t();
    for (int i = 0; i < h; i++)
    {
        int r = w * i;
        for (int j = 0; j < w; j++)
        {
            mat_t[r + j] = trans.at<float>(i, j);
        }
    }
}

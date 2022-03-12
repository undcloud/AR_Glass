#include <jni.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "LogUtils.h"
#include "opencv2/features2d.hpp"




using namespace std;
using namespace cv;

float fan;
MatND hist;
Mat nail1;
int grid_w = 50;
int grid_h = 200;
Point highest_point; // 当点手指最高点
Point highest_point1;
Point point1_trans;
Point highest_point2;

//double  * gridX1;
//double  * gridY1;
//double * gridX2;
//double * gridY2;
//gridX1 = new double [9]{double(6 * num_rows / 20), double(6 * num_rows / 20), double(6 * num_rows / 20), double(9 * num_rows / 20), double(9 * num_rows / 20), double(9 * num_rows / 20), double(12 * num_rows / 20), double(12 * num_rows / 20), double(12 * num_rows / 20)};

//    gridY1 = new double [9]{double(9 * num_cols / 20), double(10 * num_cols / 20), double(11 * num_cols / 20), double(9 * num_cols / 20), double(10 * num_cols / 20), double(11 * num_cols / 20), double(9 * num_cols / 20), double(10 * num_cols / 20), double(11 * num_cols / 20)};
//    gridX2 = new double [9]{double(6 * num_rows / 20 + 10), double(6 * num_rows / 20 + 10), double(6 * num_rows / 20 + 10) , double(9 * num_rows / 20 + 10), double(9 * num_rows / 20 + 10), double(9 * num_rows / 20 + 10), double(12 * num_rows / 20 + 10), double(12 * num_rows / 20 + 10), double(12 * num_rows / 20 + 10)};
//    gridY2 = new double [9]{double(9 * num_cols / 20 + 10), double(10 * num_cols / 20 + 10), double(11 * num_cols / 20 + 10), double(9 * num_cols / 20 + 10), double(10 * num_cols / 20 + 10), double(11 * num_cols / 20 + 10), double(9 * num_cols / 20 + 10), double(10 * num_cols / 20 + 10), double(11 * num_cols / 20 + 10)};


extern "C" {

/**
 * 灰度处理图片
 * @param env
 * @param type
 * @param pixels_ 图片的颜色数据
 * @param w 图片的宽度
 * @param h 图片的高度
 * @return
 */
JNIEXPORT jintArray JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_grayPixels(JNIEnv *env, jclass type,
                                                             jintArray pixels_, jint w, jint h) {
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);

    if (pixels == NULL) {
        return 0;
    }
    ////图片一进来时是ARGB  通过mat转换BGRA
    Mat img(h, w, CV_8UC4, pixels);
    //读取返回一个uchar数组
//    uchar* ptr = img.ptr(0);
    //获取当前CPU的钟摆时间
    double time0 = static_cast<double>(getTickCount());
    /**
     * 指针处理
     */
    //灰度处理图片
//    for(int i = 0; i < w * h; i++){
//        //R*0.299 + G * 0.587 + B * 0.114
//        uchar  grayPixel = (uchar)(ptr[4*i+2]*0.299+ptr[4*i+1]*0.587+ptr[4*i+0]*0.114);
//        ptr[4*i+0] = grayPixel;
//        ptr[4*i+1] = grayPixel;
//        ptr[4*i+2] = grayPixel;
//    }


//    /**
//     * 迭代器处理
//     */
//    //起始位置的迭代器
//   // Vec3b是向量
//    Mat_<Vec3b>::iterator it = img.begin<Vec3b>();
//    //结束为止的迭代器
//    Mat_<Vec3b>::iterator itEnd = img.end<Vec3b>();
//
//    for (; it!= itEnd; ++it) {
//        uchar temp = (uchar)((*it)[2]*0.299+(*it)[1]*0.587+(*it)[0]*0.114);
//        (*it)[0] = temp;
//        (*it)[1] = temp;
//        (*it)[2] = temp;
//    }//2054712354

    /**
     * 动态地址计算
     * 得用Vec4b
     */
    int row = img.rows;
    int col = img.cols;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            uchar temp = (uchar) (img.at<Vec4b>(i, j)[2] * 0.299
                                  + img.at<Vec4b>(i, j)[1] * 0.587
                                  + img.at<Vec4b>(i, j)[0] * 0.114);
            img.at<Vec4b>(i, j)[0] = temp;
            img.at<Vec4b>(i, j)[1] = temp;
            img.at<Vec4b>(i, j)[2] = temp;
        }
    }


    //计算运行时间
    time0 = ((double) getTickCount() - time0) / getTickFrequency();
    __android_log_print(ANDROID_LOG_INFO, "JNI", "%ld", time0);


//    //修改亮度和对比度
//    int row = img.rows;
//    int col = img.cols;
//    for (int i = 0; i < row; i++) {
//        for (int j = 0; j < col; j++) {
//            img.at<Vec4b>(i, j)[0] = img.at<Vec4b>(i, j)[0] * 0.5 + 15;
//            img.at<Vec4b>(i, j)[1] = img.at<Vec4b>(i, j)[1] * 0.5 + 15;
//            img.at<Vec4b>(i, j)[2] = img.at<Vec4b>(i, j)[2] * 0.5 + 15;
//        }
//    }
    int size = w * h;

    jintArray result = env->NewIntArray(size);

    env->SetIntArrayRegion(result, 0, size, pixels);

    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return result;
}

/**
 * 更改亮度和对比度
 * 亮度对比度的算法公式
    一副图像的亮度对比度调节属于图像的灰度线性变换，其公式如下:
    y = [x - 127.5 * (1 - B)] * k + 127.5 * (1 + B);
     x为调节前的像素值，y为调节后的像素值。
     其中B取值[-1,1]，调节亮度；
     k调节对比度，arctan(k)取值[1,89]，所以
    k = tan( (45 + 44 * c) / 180 * pi );
    其中c取值[-1,1]。通常我们用该值来设置对比度
     特别的,
     当B=0 时：y = (x - 127.5) * k + 127.5; 这时只调节对比度。
    当c=0 时，k = 1：y = x + 255 * B; 这时只调节亮度。
 * @param env
 * @param type
 * @param pixels_ 颜色数据
 * @param w 图片的宽度
 * @param h 图片的高度
 * @return
 */
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_lightPixels(JNIEnv *env, jclass type,
//                                                           jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//
//    //颜色数据转换 ARGB的颜色数据被转成BGRA
//    Mat img(h, w, CV_8UC4, pixels);
//
//    //获取行数和列数
//    int row = img.rows;
//    int col = img.cols;
////    //调节亮度 [-1,1]
////    double b = 0.5;
////    //调节对比度 [-1,1]
////    double c = 0.5;
////    double k = tan((45 + 44 * c) / 180 * M_PI);
////
////    for (int i = 0; i < row; i++) {
////        for (int j = 0; j < col; j++) {
////            img.at<Vec4b>(i, j)[0] = (uchar) ((img.at<Vec4b>(i, j)[0]-127.5 * (1 - b)) *  k + 127.5 * (1 + b));
////            img.at<Vec4b>(i, j)[1] = (uchar) ((img.at<Vec4b>(i, j)[1]-127.5 * (1 - b)) *  k + 127.5 * (1 + b));
////            img.at<Vec4b>(i, j)[2] = (uchar) ((img.at<Vec4b>(i, j)[2]-127.5 * (1 - b)) *  k + 127.5 * (1 + b));
////        }
////    }
//    //亮度
//    double b = 50;
//    //对比度 (值越小图片越灰暗)
//    double c = 0.3;
//
//    for (int i = 0; i < row; i++) {
//        for (int j = 0; j < col; j++) {
//            //Blue
//            img.at<Vec4b>(i, j)[0] = (uchar) (img.at<Vec4b>(i, j)[0] * c + b);
//            //Green
//            img.at<Vec4b>(i, j)[1] = (uchar) (img.at<Vec4b>(i, j)[1] * c + b);
//            //Red
//            img.at<Vec4b>(i, j)[2] = (uchar) (img.at<Vec4b>(i, j)[2] * c + b);
//            //Alpha
//            img.at<Vec4b>(i, j)[3] = 255;
//        }
//    }
//
//    int size = w * h;
//
//    jintArray result = env->NewIntArray(size);
//
//    env->SetIntArrayRegion(result, 0, size, pixels);
//
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//
//    return result;
//}
//
//}
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_splitMergePixels(JNIEnv *env, jclass type,
//                                                                jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//    //颜色数据转换 ARGB的颜色数据被转成BGRA
//    Mat img(h, w, CV_8UC4, pixels);
//
//    //颜色通道分离split
//    vector<Mat> outImg;
//    //颜色通道分离处理，就是把图像蓝色、绿色、红色单独分离增强，并改变图像的形态，
//    split(img, outImg);
//
//
//
//    //颜色通道合并后，颜色存储的是全蓝全红全红，会改变图片的整体形态
//    Mat src;
//    //颜色合并处理
//    merge(outImg, img);
//
//
////    增强基色色值，并不改变图片的轮廓形态
//    Mat &BMat = outImg.at(0);
//
//    //获取存储颜色数据的uchar指针
//    uchar *ptr = img.ptr(0);
//
//    //被分离的颜色的数据指针
//    uchar *b = BMat.ptr(0);
//
//
//    for (int i = 0; i < w * h; i++) {
//        ptr[4 * i + 0] = 0;
//        ptr[4 * i + 1] = 0;
//        ptr[4 * i + 2] = b[i];
//
//    }
//
//
//    int size = w * h;
//    jintArray result = env->NewIntArray(size);
//
//    env->SetIntArrayRegion(result, 0, size, pixels);
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//    return result;
//}
////颜色空间转换
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_cvtColorPixels(JNIEnv *env, jclass type,
//                                                              jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
////CV_8UC4:无符号 8 位整 数(类型为 CV_8U)表示像素亮度。
//    Mat img(h, w, CV_8UC4, pixels);
//
//
//    Mat grayImg;
//    cvtColor(img, grayImg, COLOR_BGR2GRAY);
////    cvtColor(img, img, COLOR_BGR2HLS_FULL);
////    uchar* ptr = img.ptr(0);
////
//////
////    for(int i = 0; i < w * h; i++){
////        uchar grayPixel = (uchar)(ptr[4*i]);
////
////        ptr[4 * i + 0] = grayPixel;
////        ptr[4 * i + 1] = grayPixel;
////        ptr[4 * i + 2] = grayPixel;
////    }
//
//    int size = w * h;
//    jintArray result = env->NewIntArray(size);
//    env->SetIntArrayRegion(result, 0, size, pixels);
//
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//
//    return result;
//}extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_erodeORdilate(JNIEnv *env, jclass type,
//                                                             jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//
//    Mat img(h, w, CV_8UC4, pixels);
//
////    形态作业
////
////    简而言之：一组基于形状处理图像的操作。形态操作将结构元素应用于输入图像并生成输出图像。
////    最基本的形态作用是：侵蚀和扩张。它们有广泛的用途，即：
////    消除噪音
////    隔离单个元素并连接图像中的不同元素。
////    查找图像中的强度凸点或孔
//
//
//
//
////    我们可以为我们的内核选择三种形状：
////    矩形框：MORPH_RECT
////    十字架：MORPH_CROSS
////    椭圆：MORPH_ELLIPSE
//
//    Mat pp = getStructuringElement(MORPH_RECT, Size(10, 10));
//    //腐蚀（侵蚀）图像，向内有坍缩，10*10为基准，消除周边的颜色，用中心的颜色去填充周边颜色。
////    这个操作是扩张的姊妹。它计算给定内核区域的局部最小值。
////    当内核在图像上扫描时，我们计算由重叠的最小像素值，并用该最小值替换锚点下的图像像素。BB
////    对于扩张的例子，我们可以将侵蚀算子应用于原始图像
//
//
////    src：源图像
////    erosion_dst：输出图像
////    element：这是我们将用来执行操作的内核。如果我们不指定，默认是一个简单的3x3矩阵。否则，我们可以指定它的形状。为此，我们需要使用函数cv :: getStructuringElement：
//
//    erode(img, img, pp);
//    //扩张
//
////    扩张，向外扩张
////
////    该操作包括将图像与某些内核（B）进行卷积，其可以具有任何形状或尺寸，通常为正方形或圆形。AB
////    内核具有定义的锚点，通常是内核的中心。B
////    当内核在图像上扫描时，我们计算由B重叠的最大像素值，并用该最大值替换锚点位置中的图像像素。您可以推断，这种最大化的操作会使图像中的亮区“增长”（因此称为扩张）。
////    dilate(img,img,pp);
//    int size = w * h;
//    jintArray result = env->NewIntArray(size);
//    env->SetIntArrayRegion(result, 0, size, pixels);
//
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//    return result;
//}
//
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_cvtColorPixels1(JNIEnv *env, jclass type,
//                                                               jstring path_) {
//    const char *path = env->GetStringUTFChars(path_, 0);
//
//    Mat img = imread(path);
//
//    if (!img.data) {
//
//    }
//
////    namedWindow("原图像", WINDOW_AUTOSIZE);
////    imshow("原图像", img);
//
//    Mat grayImg;
//
//    cvtColor(img, grayImg, COLOR_BGRA2GRAY);
//
//    int h = grayImg.rows;
//    int w = grayImg.cols;
//    int size = h * w;
//
//    uchar *g = grayImg.ptr(0);
//
//    for (int i = 0; i < size; ++i) {
//
//    }
//
//
//    jintArray result = env->NewIntArray(size);
//
//
//    env->ReleaseStringUTFChars(path_, path);
//}
//
////增加图片亮度
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_lightImage(JNIEnv *env, jclass type,
//                                                          jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//
//    Mat img(h, w, CV_8UC4, pixels);
//
//    uchar *ptr = img.ptr(0);
////    saturate_cast：这个函数保证值类型不溢出
//    for (int i = 0; i < w * h; ++i) {
//        ptr[4 * i + 0] = saturate_cast<uchar>(ptr[4 * i + 0] + 50);
//        ptr[4 * i + 1] = saturate_cast<uchar>(ptr[4 * i + 1] + 50);
//        ptr[4 * i + 2] = saturate_cast<uchar>(ptr[4 * i + 2] + 50);
//    }
//
//    int size = w * h;
//    jintArray result = env->NewIntArray(size);
//    env->SetIntArrayRegion(result, 0, size, pixels);
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//    return result;
//}
//
////滤波器
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_blurImage(JNIEnv *env, jclass type,
//                                                         jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//
////    Mat img(h, w, CV_8UC4, pixels);
//    //线性滤波：方框滤波、均值滤波、高斯滤波（高斯函数作为滤波函数时做滤波操作，高斯低通是做模糊操作）
//    //非线性滤波：中值滤波、双边滤波。
//
//    /**
//      * 方框滤波
//      * 第一个参数：输入的图片
//      * 第二个参数：输出的图片
//      * 第三个参数：图片的深度（存储每个像素所用的位数）一般情况使用-1也就是原有的图像深度
//      * 第四个参数：核心的大小
//      * 第五个参数：锚点的位置，就是我们要进行处理的点，默认值（-1，-1）表示锚点在核的中心
//      * 第六个参数：normalize默认值为true
//      * 第七个参数：边界模式，默认值BORDER_DEFAULT
//      */
////    boxFilter(img,img,-1,Size(30,30));
//
///**
// * 均值滤波（归一化之后又进行了方框滤波）
// * 第一个参数：输入的图片
// * 第二个参数：输出的图片
// * 第三个参数：核心的大小
// * 第四个参数：锚点的位置，就是我们要进行处理的点，默认值（-1，-1）表示锚点在核的中心
// * 第五个参数：边界模式，默认值BORDER_DEFAULT
// */
//
////    blur(img,img,Size(30,30));
//
//    /**
//     * 高斯滤波
//     * 第一个参数：传入的图片
//     * 第二个参数：传出的图片
//     * 第三个参数：核心（必须是正数和奇数）
//     * 第四个参数：sigmaX代表高斯函数在x方向的标准偏差
//     * 第五个参数：sigmaY代表高斯函数在Y方向的标准偏差，有默认值为0
//     * 第六个参数：边界模式，使用默认值BORDER_DEFAULT
//     */
////    GaussianBlur(img,img,Size(31,31),0);
//
//    /**
//     * 中值滤波,孔径范围内的所有像素进行排序，然后取中位数，赋值给核心。
//     * 第一个参数：传入的图片
//     * 第二个参数：传出的图片
//     * 第三个参数：孔径的线性尺寸，必须是大于1的奇数
//     */
////    medianBlur(img,img,31);
//
//
//
//    /**
//      * 双边滤波
//      * 第一个参数：传入的图片(必须是CV_8UC1或者CV_8UC3)
//      * 第二个参数：传出的图片
//      * 第三个参数：每个像素领域的直径
//      * 第四个参数：sigmaColor，这个值越大，该像素领域内会有更广的颜色被混合到一起
//      * 第五个参数：sigmaSpace，这个值越大，越远的像素会互相影响，第三个参数大于0时，领域的大小和这个值无关，否则成正比
//      * 第六个参数：使用默认值BORDER_DEFAULT
//      */
////    vector<Mat> outimg;
////    split(img,outimg);
////
////    uchar* imgc3=NULL;
////    for (int i = 0; i <w*h ; ++i) {
////        imgc3[3*i+0]=outimg.at(0).ptr(0)[i];
////        imgc3[3*i+1]=outimg.at(1).ptr(0)[i];
////        imgc3[3*i+2]=outimg.at(2).ptr(0)[i];
////    }
////    Mat img2c3(h,w,CV_8UC3,imgc3);
////    Mat reimg(h,w,CV_8UC3);
//    Mat img(h, w, CV_8UC3, pixels);
////
//    Mat out(h,w,CV_8UC4);
//    bilateralFilter(img,out,25,25*2,25/2);
//
////    jint* out = (jint *) reimg.data;
////    jint* re =NULL;
////    for (int i = 0; i <w*h ; ++i) {
////        re[4*i+0]=out[3*i+0];
////        re[4*i+1]=out[3*i+1];
////        re[4*i+2]=out[3*i+2];
////        re[4*i+2]=outimg.at(4).ptr(0)[i];
////    }
//    jint *re = (jint *) out.data;
//    int size = w * h;
//    jintArray array = env->NewIntArray(size);
//    env->SetIntArrayRegion(array, 0, size, re);
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//
//    return array;
//}
//
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_morpImage(JNIEnv *env, jclass type,
//                                                         jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//
//    Mat img(h,w,CV_8UC4,pixels);
////    Mat img(h,w,CV_8UC1,pixels);
//
//    vector<Mat> outchannl;
//    split(img,outchannl);
//    Mat out3img;
//    cvtColor(img,out3img,COLOR_BGRA2BGR);
//
//
//    Mat coreMat = getStructuringElement(MORPH_RECT,Size(10,10));
//    /**
//    * 腐蚀
//    * 参数1：输入的图片，图像深度CV_8U，CV_16U，CV_16S，CV_32F，CV_64F
//    * 参数2：输出的图片
//    * 参数3：核心
//    * 参数4：锚点
//    * 参数5：运算的次数，次数是基于图片的，每次都是在上次的结果上去操作
//    * 参数6：边界模式
//    * 参数7：边界值为常数时，会有默认值，最后两个参数一般不做操作
//    */
////    erode(img,img,coreMat);
//    /**
//       * 膨胀
//       * 参数1：输入的图片，图像深度CV_8U，CV_16U，CV_16S，CV_32F，CV_64F
//       * 参数2：输出的图片
//       * 参数3：核心
//       * 参数4：锚点
//       * 参数5：运算的次数，次数是基于图片的，每次都是在上次的结果上去操作
//       * 参数6：边界模式
//       * 参数7：边界值为常数时，会有默认值，最后两个参数一般不做操作
//       */
////    dilate(img,img,coreMat);
//
//    /**
//     * 形态学滤波函数 morphologyEx()
//     * 参数1：输入的图片
//     * 参数2；输出的图片
//     * 参数3：形态学运算的类型    MORPH_ERODE  //腐蚀
//     *                        MORPH_DILATE//膨胀
//     *                        MORPH_OPEN  开运算
//     *                        MORPH_CLOSE 闭运算
//     *                        MORPH_GRADIENT//梯度运算
//     *                        MORPH_TOPHAT //顶帽
//     *                        MORPH_BLACKHAT //黑帽
//     *                        MORPH_HITMISS   assert src.type() == CV_8UC1 //击中击不中
//     * 参数4：核心，当这个值为NULL时，默认为3*3的核
//     * 参数5：锚点
//     * 参数6：迭代次数
//     * 参数7：边界模式
//     * 参数8：边界值为常数时，会有默认值
//     */
//    morphologyEx(out3img,out3img,MORPH_BLACKHAT,coreMat);
//
//    uchar * ptr = img.ptr(0);
//    uchar *out = out3img.ptr(0);
//
//    for (int i = 0; i < h * w; ++i) {
//        ptr[4*i+0] = out[3*i+0];
//        ptr[4*i+1] = out[3*i+1];
//        ptr[4*i+2] = out[3*i+2];
//    }
//
////    Mat img(h,w,CV_8UC4,pixels);
////
////    Mat out;
////    cvtColor(img,out,COLOR_BGR2GRAY);
////
////    //核
////    Mat coreMat = getStructuringElement(MORPH_RECT,Size(10,10));
////    //击中击不中变换运算 src.type() == CV_8UC1
////    morphologyEx(out,out,MORPH_HITMISS,coreMat);
////
////    uchar *ptr = img.ptr(0);
////    uchar *outPtr = out.ptr(0);
////    int size = w*h;
////    for (int i = 0; i < size; ++i) {
////        ptr[4*i+0] = outPtr[i];
////        ptr[4*i+1] = outPtr[i];
////        ptr[4*i+2] = outPtr[i];
////    }
//
//    int size = w * h;
//    jintArray result = env->NewIntArray(size);
//    env->SetIntArrayRegion(result,0,size,pixels);
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//    return result;
//}
//
//
////边缘检测
//extern "C"
//JNIEXPORT jintArray JNICALL
//Java_com_jiangdg_usbcamera_view_USBCameraActivity_edgeDetection(JNIEnv *env, jclass type,
//                                                             jintArray pixels_, jint w, jint h) {
//    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
//
//    if(pixels == NULL){
//        return 0;
//    }
//
//    Mat img(h,w,CV_8UC4,pixels);
//    Mat outImg;
//    cvtColor(img,outImg,COLOR_BGR2GRAY);//单通道
//
//    /**
//   * canny算子 单通道_dst.getObj() != _src.getObj() || _src.type() == CV_8UC1
//   * 第一个参数，输入图像，需为单通道8位图像。
//   * 第二个参数，输出的边缘图，需要和源图片有一样的尺寸和类型。
//   * 第三个参数，第一个滞后性阈值。
//   * 第四个参数，第二个滞后性阈值。
//   * 第五个参数，表示孔径大小，其有默认值3。
//   * 第六个参数，计算图像梯度幅值的标识，有默认值false。
//   * 需要注意的是，这个函数阈值1和阈值2两者的小者用于边缘连接，
//   * 而大者用来控制强边缘的初始段，推荐的高低阈值比在2:1到3:1之间
//   */
//    //滤波
////    blur(outImg,outImg,Size(3,3));
////    //边缘检测
//    Canny(outImg,outImg,3,9);
////    Mat out;
////    out = Scalar::all(0);
////    img.copyTo(out,outImg);
//
//
//
//    /**
//     * sobel算子 单通道
//     * 第一个参数，输入图像
//     * 第二个参数，目标图像
//     * 第三个参数，输出图像的深度
//            使用CV_8U, 取ddepth =-1/CV_16S/CV_32F/CV_64F
//            使用CV_16U/CV_16S, 取ddepth =-1/CV_32F/CV_64F
//            使用CV_32F, 取ddepth =-1/CV_32F/CV_64F
//            使用CV_64F, 取ddepth = -1/CV_64F
//     * 第四个参数，x 方向上的差分阶数。
//     * 第五个参数，y方向上的差分阶数。
//     * 第六个参数，核的大小;必须取1，3，5或7。
//     * 第七个参数，计算导数值时可选的缩放因子，默认值是1，表示默认情况下是没有应用缩放的。
//     * 第八个参数，表示在结果存入目标图（第二个参数dst）之前可选的delta值，有默认值0。
//     * 第九个参数，边界模式，默认值为BORDER_DEFAULT。
//     * 一般情况下，都是用ksize x ksize内核来计算导数的。
//     * 然而，有一种特殊情况——当ksize为1时，往往会使用3 x 1或者1 x 3的内核。
//     * 且这种情况下，并没有进行高斯平滑操作。
//     */
//
//
//    //x方向sobel梯度
////    Sobel(outImg,outImg,CV_8U,1,0);
////    y方向sobel梯度
////    Sobel(outImg,outImg,CV_8U,0,1);
////    Mat outimgX,outimgY,outXY;
////    //x方向sobel梯度
////    Sobel(outImg,outimgX,CV_8U,1,0);
////    //y方向sobel梯度
////    Sobel(outImg,outimgY,CV_8U,0,1);
////    //合并梯度
////    addWeighted(outimgX,0.5,outimgY,0.5,0,outXY);
//
//    /**
//       * laplacian函数
//       * 第一个参数，输入图像，需为单通道8位图像。
//       * 第二个参数，输出的边缘图，需要和源图片有一样的尺寸和通道数。
//       * 第三个参数，目标图像的深度。
//       * 第四个参数，用于计算二阶导数的滤波器的孔径尺寸，大小必须为正奇数，且有默认值1。
//       * 第五个参数，计算拉普拉斯值的时候可选的比例因子，有默认值1。
//       * 第六个参数，表示在结果存入目标图（第二个参数dst）之前可选的delta值，有默认值0。
//       * 第七个参数，边界模式，默认值为BORDER_DEFAULT。
//       */
//    Laplacian(outImg,outImg,CV_8U);
//
//    uchar *ptr = img.ptr(0);
//
//    uchar *outPtr = outImg.ptr(0);
//
//    for (int i = 0; i < w * h; ++i) {
//        ptr[4*i+0] = outPtr[i];
//        ptr[4*i+1] = outPtr[i];
//        ptr[4*i+2] = outPtr[i];
//    }
//
//    int size = w*h;
//    jintArray result = env->NewIntArray(size);
//    env->SetIntArrayRegion(result,0,size,pixels);
//
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);
//    return result;
//}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_nativeRgba(JNIEnv *env, jclass type,
                                                             jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;

    Mat imgT(height, width, CV_8UC4);// 4通道 fan
    Mat imgF(height, width, CV_8UC4);
//    //图片倒置
    transpose(img, imgT);
    /**
   * 重定义图片大小
   * 第三个参数dsize：size格式的图片大小
   * 第四个参数dx：x方向的图片缩放比
   * 第五个参数dy：y方向的缩放比
   * 第六个参数：插值方式，一般情况使用默认值（线性插值）
   * dsize不为0的时候，dx和dy是无效的
   *
   */
    resize(imgT, imgF, Size(width, height));
    /**
     * 翻转图片
     * 第三个参数，0以x轴进行翻转，1以y轴翻转，-1以xy同时翻转
     */
    flip(imgF, img, 1);


    Mat out;
    cvtColor(img, out, COLOR_RGBA2GRAY);
//
//    blur(out,out,Size(3,3));
////    //边缘检测
//    Canny(out,out,3,9);
//    Mat outImg;
//    outImg = Scalar::all(0);
//    img.copyTo(outImg,out);

    Mat outimgX, outimgY, outXY;
    //x方向sobel梯度
    Sobel(out, outimgX, CV_8U, 1, 0);
    //y方向sobel梯度
    Sobel(out, outimgY, CV_8U, 0, 1);
    //合并梯度
    addWeighted(outimgX, 0.5, outimgY, 0.5, 0, outXY);


    uchar *ptr = img.ptr(0);

    uchar *outPtr = outXY.ptr(0);
    // 这里将一通道图片 转为 4通道图片，但是不知道为什么要这样做，不知道如何保留在4通道下保持灰度信息。
    for (int i = 0; i < img.rows * img.cols; ++i) {
        ptr[4 * i + 0] = outPtr[i];
        ptr[4 * i + 1] = outPtr[i];
        ptr[4 * i + 2] = outPtr[i];
    }

}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_preprocess(JNIEnv *env, jclass type,
                                                             jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;
    Mat imgT(height, width, CV_8UC4);// 4通道 fan
    Mat imgF(height, width, CV_8UC4);
//    //图片倒置
    transpose(img, imgT);
    /**
     *
    * 原图是否被污染了？
     *
   * 重定义图片大小
   * 第三个参数dsize：size格式的图片大小
   * 第四个参数dx：x方向的图片缩放比
   * 第五个参数dy：y方向的缩放比
   * 第六个参数：插值方式，一般情况使用默认值（线性插值）
   * dsize不为0的时候，dx和dy是无效的
   *
   */
    resize(imgT, imgF, Size(width, height));// 这个resize必须加，要不然不能运行
    /**
     * 翻转图片
     * 第三个参数，0以x轴进行翻转，1以y轴翻转，-1以xy同时翻转
     */
    flip(imgF, img, 1);
    cvtColor(img, img, COLOR_RGBA2RGB);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_gridShow(JNIEnv *env, jclass type,
                                                           jlong jrgba, jint width, jint height,
                                                           jdoubleArray x1, jdoubleArray y1,
                                                           jdoubleArray x2, jdoubleArray y2) {
/*
 * 皮肤颜色采集框 方式一
 */
    Mat &img = *(Mat *) jrgba;

    jdouble *jx1 = env->GetDoubleArrayElements(x1, JNI_FALSE);
    jdouble *jy1 = env->GetDoubleArrayElements(y1, JNI_FALSE);
    jdouble *jx2 = env->GetDoubleArrayElements(x2, JNI_FALSE);
    jdouble *jy2 = env->GetDoubleArrayElements(y2, JNI_FALSE);


    for (int n = 0; n <= 8; n++) {
        rectangle(img, Point(jx1[n], jy1[n]), Point(jx2[n], jy2[n]), Scalar(1));
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_gridShow2(JNIEnv *env, jclass type,
                                                            jlong jrgba, jint width, jint height) {
/*
 * 皮肤颜色采集框 方式二
 */
    Mat &img = *(Mat *) jrgba;
    rectangle(img, Point(width / 2 - grid_w / 2, height / 2 - grid_h / 2),
              Point(width / 2 + grid_w / 2, height / 2 + grid_h / 2), Scalar(1));
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_handHistogram(JNIEnv *env, jclass type,
                                                                jlong jrgba, jint width,
                                                                jint height, jdoubleArray x1,
                                                                jdoubleArray y1, jdoubleArray x2,
                                                                jdoubleArray y2) {
    /**
     * 计算9个矩形内的颜色HSV统计直方图
     */

    jdouble *jx1 = env->GetDoubleArrayElements(x1, JNI_FALSE);
    jdouble *jy1 = env->GetDoubleArrayElements(y1, JNI_FALSE);
    jdouble *jx2 = env->GetDoubleArrayElements(x2, JNI_FALSE);
    jdouble *jy2 = env->GetDoubleArrayElements(y2, JNI_FALSE);

    Mat &img = *(Mat *) jrgba;

    //Quantize the hue to 60 levels  https://blog.csdn.net/skeeee/article/details/8979811
    //and the saturation to 64 levels
    int hbins = 180, sbins = 256;
    int histSize[] = {hbins, sbins};
    //hue varies from 0 to 179
    float hranges[] = {0, 180};
    //saturation varies from 0 to 255
    float sranges[] = {0, 256};
    const float *ranges[] = {hranges, sranges};
    //two channels 0th,1th
    int channels[] = {0, 1};
    Mat test[9];
    for (int i = 0; i <= 8; i++) {
        test[i] = img(Rect(jx1[i], jy1[i], jx2[i] - jx1[i], jy2[i] - jy1[i]));
        cvtColor(test[i], test[i], COLOR_RGB2HSV);
//        cvtColor(test[i], test[i], COLOR_RGBA2RGB);
//        cvtColor(test[i], test[i], COLOR_RGB2HSV);


        calcHist(&test[i], 1, channels, Mat(), hist, 2, histSize, ranges, true, true);
    }
    normalize(hist, hist, 0, 255, NORM_MINMAX);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_handHistogram2(JNIEnv *env, jclass type,
                                                                 jlong jrgba, jint width,
                                                                 jint height) {
    Mat &img = *(Mat *) jrgba;

    //Quantize the hue to 60 levels  https://blog.csdn.net/skeeee/article/details/8979811
    //and the saturation to 64 levels
    int hbins = 180, sbins = 256;
    int histSize[] = {hbins, sbins};
    //hue varies from 0 to 179
    float hranges[] = {0, 180};
    //saturation varies from 0 to 255
    float sranges[] = {0, 256};
    const float *ranges[] = {hranges, sranges};
    //two channels 0th,1th
    int channels[] = {0, 1};
    Mat temp;

    temp = img(Rect(int(width / 2 - grid_w / 2), int(height / 2 - grid_h / 2), int(grid_w),
                    int(grid_h)));
    cvtColor(temp, temp, COLOR_RGB2HSV);
//    cvtColor(temp, temp, COLOR_RGBA2RGB);
//    cvtColor(temp, temp, COLOR_RGB2HSV);


    calcHist(&temp, 1, channels, Mat(), hist, 2, histSize, ranges, true, true);
    normalize(hist, hist, 0, 255, NORM_MINMAX);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_getNail(JNIEnv *env, jclass type,
                                                          jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;

    //根据直方图统计　过滤掉　无关像素
    Mat hsv;
    cvtColor(img, hsv, COLOR_RGB2HSV);
//    cvtColor(img, hsv, COLOR_RGBA2RGB);
//    cvtColor(img, hsv, COLOR_RGB2HSV);


    Mat dst;
    int channels[] = {0, 1};
    float hranges[] = {0, 180};
    //saturation varies from 0 to 255
    float sranges[] = {0, 255};
    const float *ranges[] = {hranges, sranges};
    calcBackProject(&hsv, 1, channels, hist, dst, ranges);
    Mat disc;
    disc = getStructuringElement(MORPH_ELLIPSE, Size(31, 31));
    filter2D(dst, dst, -1, disc);
    threshold(dst, dst, 180, 255, THRESH_BINARY);
    Mat planes[] = {dst, dst, dst};
    Mat dst3;
    merge(planes, 3, dst3);
    Mat and_dst;
    bitwise_and(img, dst3, and_dst);

    //腐蚀
    Mat erodeMat;
    erode(and_dst, erodeMat, Mat(), Point(-1, -1), 2);

    //膨胀
    Mat dilateMat;
    dilate(erodeMat, dilateMat, Mat(), Point(-1, -1), 2);

    //得到轮廓
    Mat matGray;
    cvtColor(dilateMat, matGray, COLOR_RGB2GRAY);
    Mat threDst;
    threshold(matGray, threDst, 150, 255, 0);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy; // 放了4维int向量//一般在轮廓查找和轮廓绘制使用
    findContours(threDst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return;
    }

    // 得到轮廓
    //    file:///home/arfan/%E4%B8%8B%E8%BD%BD/3.3.0/dd/d9d/segment_objects_8cpp-example.html#a4
    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int idx = 0, largestComp = 0;
    double maxArea = 0;
    for (; idx >= 0; idx = hierarchy[idx][0]) {
        const vector<Point> &c = contours[idx];
        double area = fabs(contourArea(Mat(c)));
        if (area > maxArea) {
            maxArea = area;
            largestComp = idx;
        }
    }
    Scalar color(0, 0, 255);
//    drawContours( img, contours, largestComp, color, FILLED, LINE_8, hierarchy );

    // 计算中心
    Moments moments1 = moments(contours[largestComp]);
    int cx, cy;
    if (moments1.m00 != 0) {
        cx = int(moments1.m10 / moments1.m00);
        cy = int(moments1.m01 / moments1.m00);
    } else {
        return;
    }

    // 计算轮廓的最高点
//    circle(img, Point(cx, cy),  5, Scalar(255, 0, 255), -1);
    // Detected contours. Each contour is stored as a vector of points (e.g. std::vector<std::vector<cv::Point> >).
    int min_y = height - 1;

    for (int i = 0; i <= contours[largestComp].size() - 1; i++) {
        if (contours[largestComp].at(i).y < min_y) {
            min_y = contours[largestComp].at(i).y;
            highest_point = contours[largestComp].at(i);
        }
    }


    circle(img, highest_point, 5, Scalar(255, 0, 255), -1);
//      circle(img, Point(highest_point.x,highest_point.y-25),  5, Scalar(255, 0, 255), -1);


}


extern "C"
JNIEXPORT jintArray JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_getNail2(JNIEnv *env, jclass type,
                                                           jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;

//根据直方图统计　过滤掉　无关像素
    Mat hsv;
    cvtColor(img, hsv, COLOR_RGB2HSV);
//    cvtColor(img, hsv, COLOR_RGBA2RGB);
//    cvtColor(img, hsv, COLOR_RGB2HSV);


    Mat dst;
    int channels[] = {0, 1};
    float hranges[] = {0, 180};
//saturation varies from 0 to 255
    float sranges[] = {0, 255};
    const float *ranges[] = {hranges, sranges};
    calcBackProject(&hsv, 1, channels, hist, dst, ranges);
    Mat disc;
    disc = getStructuringElement(MORPH_ELLIPSE, Size(31, 31));
    filter2D(dst, dst, -1, disc);
    threshold(dst, dst, 180, 255, THRESH_BINARY);
    Mat planes[] = {dst, dst, dst};
    Mat dst3;
    merge(planes, 3, dst3);
    Mat and_dst;
    bitwise_and(img, dst3, and_dst);

//腐蚀
    Mat erodeMat;
    erode(and_dst, erodeMat, Mat(), Point(-1, -1), 2);

//膨胀
    Mat dilateMat;
    dilate(erodeMat, dilateMat, Mat(), Point(-1, -1), 2);

//得到轮廓
    Mat matGray;
    cvtColor(dilateMat, matGray, COLOR_RGB2GRAY);
    Mat threDst;
    threshold(matGray, threDst, 150, 255, 0);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy; // 放了4维int向量//一般在轮廓查找和轮廓绘制使用
    findContours(threDst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        jintArray array = env->NewIntArray(2);
        jint *elems = env->GetIntArrayElements(array, NULL);
        elems[0] = 0;
        elems[1] = 0;
        env->ReleaseIntArrayElements(array, elems, 0); // 取值 零(0) 时，更新数组并释放所有元素; http://blog.sina.com.cn/s/blog_4c451e0e0101339r.html
        return array;
    }

// 得到轮廓
//    file:///home/arfan/%E4%B8%8B%E8%BD%BD/3.3.0/dd/d9d/segment_objects_8cpp-example.html#a4
// iterate through all the top-level contours,
// draw each connected component with its own random color
    int idx = 0, largestComp = 0;
    double maxArea = 0;
    for (; idx >= 0; idx = hierarchy[idx][0]) {
        const vector<Point> &c = contours[idx];
        double area = fabs(contourArea(Mat(c)));
        if (area > maxArea) {
            maxArea = area;
            largestComp = idx;
        }
    }
    Scalar color(0, 0, 255);
//    drawContours( img, contours, largestComp, color, FILLED, LINE_8, hierarchy );

// 计算中心
    Moments moments1 = moments(contours[largestComp]);
    int cx, cy;
    if (moments1.m00 != 0) {
        cx = int(moments1.m10 / moments1.m00);
        cy = int(moments1.m01 / moments1.m00);
    } else {
        jintArray array = env->NewIntArray(2);
        jint *elems = env->GetIntArrayElements(array, NULL);
        elems[0] = 0;
        elems[1] = 0;
        env->ReleaseIntArrayElements(array, elems, 0); // 取值 零(0) 时，更新数组并释放所有元素; http://blog.sina.com.cn/s/blog_4c451e0e0101339r.html
        return array;
    }

// 计算轮廓的最高点
//    circle(img, Point(cx, cy),  5, Scalar(255, 0, 255), -1);
// Detected contours. Each contour is stored as a vector of points (e.g. std::vector<std::vector<cv::Point> >).
    int min_y = height - 1;

    for (int i = 0; i <= contours[largestComp].size() - 1; i++) {
        if (contours[largestComp].at(i).y < min_y) {
            min_y = contours[largestComp].at(i).y;
            highest_point = contours[largestComp].at(i);
        }
    }

//    circle(img, highest_point, 5, Scalar(255, 0, 255), -1);
//      circle(img, Point(highest_point.x,highest_point.y-25),  5, Scalr(255, 0, 255), -1);

    jintArray array = env->NewIntArray(2);
    jint *elems = env->GetIntArrayElements(array, NULL);
    elems[0] = highest_point.x;
    elems[1] = highest_point.y;
    env->ReleaseIntArrayElements(array, elems, 0); // 取值 零(0) 时，更新数组并释放所有元素; http://blog.sina.com.cn/s/blog_4c451e0e0101339r.html
    return array;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_nail1(JNIEnv *env, jclass type,
                                                        jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;
//    if(highest_point.y < 25 ){
//        highest_point1 = highest_point;
//    }
//    else{
//        highest_point1 = Point(highest_point.x, highest_point.y-25);
//    }
    highest_point1 = highest_point;

    nail1 = img(Rect(0, 0, width, highest_point1.y));

//    resize(nail1,img,Size(width,height));
}

// 特征点匹配，找到第一个点的位置  fan
extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_nail2(JNIEnv *env, jclass type,
                                                        jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;
    highest_point2 = highest_point;

    // https://blog.csdn.net/sunnyblogs/article/details/107570279
    // Ptr<ORB> orb = ORB::create ( 500, 1.2f, 8, 8, 0, 2, ORB::HARRIS_SCORE,31,20 );//nfeatures:表示取的特征点数量，float scaleFactor = 1.2f   ：表示每一层与上一层的比例关系
    // Ptr<AKAZE> orb = AKAZE::create ();//nfeatures:表示取的特征点数量，float scaleFactor = 1.2f   ：表示每一层与上一层的比例关系
    // https://blog.csdn.net/hust_bochu_xuchao/article/details/52153167
    // FeatureDetector detector = FeatureDetector.create(FeatureDetector.ORB);
    Ptr<BRISK> brisk = BRISK::create();
    // Ptr<FastFeatureDetector> orb = FastFeatureDetector::create (); //报错
    // Ptr<GFTTDetector> orb = GFTTDetector::create ();// 报错
    // Ptr<SimpleBlobDetector> orb = SimpleBlobDetector::create ();// 报错
    // OpenCV(3.4.1) Error: The function/feature is not implemented () in detectAndCompute, file /home/arfan/opencv-3.4.1/modules/features2d/src/feature2d.cpp, line 154
    // terminate called after throwing an instance of 'cv::Exception'
    // what():  OpenCV(3.4.1) /home/arfan/opencv-3.4.1/modules/features2d/src/feature2d.cpp:154: error: (-213)  in function detectAndCompute

    std::vector<KeyPoint> keypoints_1, keypoints_2;

    brisk->detect(nail1, keypoints_1);
    brisk->detect(img, keypoints_2);

    //-- Step 2: Calculate descriptors (feature vectors)

    Mat descriptors_1, descriptors_2;

    brisk->compute(nail1, keypoints_1, descriptors_1);
    brisk->compute(img, keypoints_2, descriptors_2);

    // imshow("descriptors_1", descriptors_1);
    // imshow("descriptors_2",descriptors_2);

    //-- Step 3: Matching descriptor vectors with a brute force matcher
    BFMatcher matcher(NORM_HAMMING);
    std::vector<DMatch> matches;
    matcher.match(descriptors_1, descriptors_2, matches);

    double min_dist = 10000, max_dist = 0;//定义距离
    for (int i = 0; i < descriptors_1.rows; ++i)//遍历
    {
        double dist = matches[i].distance;
        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }

    printf("Max dist: %f\n", max_dist);
    printf("Min dist: %f\n", min_dist);

    //第五步：根据最小距离，对匹配点进行筛选，
    //当描述自之间的距离大于两倍的min_dist，即认为匹配有误，舍弃掉。
    //但是有时最小距离非常小，比如趋近于0了，所以这样就会导致min_dist到2*min_dist之间没有几个匹配。
    // 所以，在2*min_dist小于30的时候，就取30当上限值，小于30即可，不用2*min_dist这个值了
    std::vector<DMatch> good_matches;
    for (int j = 0; j < descriptors_1.rows; ++j) {
        if (matches[j].distance <= max(2 * min_dist, 30.0)) {
            good_matches.push_back(matches[j]);
        }

    }

    //第六步：绘制匹配结果

//    Mat img_match;//所有匹配点图
    //这里看一下drawMatches()原型参数，简单用法就是：图1，图1关键点，图2，图2关键点，匹配数组，承接图像，后面的有默认值
    /*
        CV_EXPORTS_W void drawMatches( InputArray img1,
                                    const std::vector<KeyPoint>& keypoints1,
                                    InputArray img2,
                                    const std::vector<KeyPoint>& keypoints2,
                                    const std::vector<DMatch>& matches1to2,
                                    InputOutputArray outImg,
                                    const Scalar& matchColor=Scalar::all(-1),
                                    const Scalar& singlePointColor=Scalar::all(-1),
                                    const std::vector<char>& matchesMask=std::vector<char>(),
                                    int flags=DrawMatchesFlags::DEFAULT );
    */
//    drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_match);
//    imshow("所有匹配点对", img_match);
//
//    Mat img_goodmatch;//筛选后的匹配点图
//    drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_goodmatch);
//    imshow("筛选后的匹配点对", img_goodmatch);

    //-- Localize the object from img_1 in img_2
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for (int i = 0; i < good_matches.size(); i++) {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints_1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints_2[good_matches[i].trainIdx].pt);
    }

    if (obj.size() <= 3) {
        printf("no enough point");
        return;
    }

    Mat H = findHomography(obj, scene, CV_RANSAC);

    vector<Point2f> points, points_trans;
    points.push_back(highest_point1);
    perspectiveTransform(points, points_trans, H);

    point1_trans = points_trans[0];

    circle(img, points_trans[0], 5, Scalar(0, 0, 255), -1);
//    highest_point2;
//    circle(img, highest_point2,  5, Scalar(0, 0, 255), -1);

}


// 特征点匹配，找到第一个点的位置  fan
extern "C"
JNIEXPORT jintArray JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_nail2_1(JNIEnv *env, jclass type,
                                                        jlong jrgba, jint width, jint height) {
    Mat &img = *(Mat *) jrgba;
    highest_point2 = highest_point;

    // https://blog.csdn.net/sunnyblogs/article/details/107570279
    // Ptr<ORB> orb = ORB::create ( 500, 1.2f, 8, 8, 0, 2, ORB::HARRIS_SCORE,31,20 );//nfeatures:表示取的特征点数量，float scaleFactor = 1.2f   ：表示每一层与上一层的比例关系
    // Ptr<AKAZE> orb = AKAZE::create ();//nfeatures:表示取的特征点数量，float scaleFactor = 1.2f   ：表示每一层与上一层的比例关系
    // https://blog.csdn.net/hust_bochu_xuchao/article/details/52153167
    // FeatureDetector detector = FeatureDetector.create(FeatureDetector.ORB);
    Ptr<BRISK> brisk = BRISK::create();
    // Ptr<FastFeatureDetector> orb = FastFeatureDetector::create (); //报错
    // Ptr<GFTTDetector> orb = GFTTDetector::create ();// 报错
    // Ptr<SimpleBlobDetector> orb = SimpleBlobDetector::create ();// 报错
    // OpenCV(3.4.1) Error: The function/feature is not implemented () in detectAndCompute, file /home/arfan/opencv-3.4.1/modules/features2d/src/feature2d.cpp, line 154
    // terminate called after throwing an instance of 'cv::Exception'
    // what():  OpenCV(3.4.1) /home/arfan/opencv-3.4.1/modules/features2d/src/feature2d.cpp:154: error: (-213)  in function detectAndCompute

    std::vector<KeyPoint> keypoints_1, keypoints_2;

    brisk->detect(nail1, keypoints_1);
    brisk->detect(img, keypoints_2);

    //-- Step 2: Calculate descriptors (feature vectors)

    Mat descriptors_1, descriptors_2;

    brisk->compute(nail1, keypoints_1, descriptors_1);
    brisk->compute(img, keypoints_2, descriptors_2);

    // imshow("descriptors_1", descriptors_1);
    // imshow("descriptors_2",descriptors_2);

    //-- Step 3: Matching descriptor vectors with a brute force matcher
    BFMatcher matcher(NORM_HAMMING);
    std::vector<DMatch> matches;
    matcher.match(descriptors_1, descriptors_2, matches);

    double min_dist = 10000, max_dist = 0;//定义距离
    for (int i = 0; i < descriptors_1.rows; ++i)//遍历
    {
        double dist = matches[i].distance;
        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }

    printf("Max dist: %f\n", max_dist);
    printf("Min dist: %f\n", min_dist);

    //第五步：根据最小距离，对匹配点进行筛选，
    //当描述自之间的距离大于两倍的min_dist，即认为匹配有误，舍弃掉。
    //但是有时最小距离非常小，比如趋近于0了，所以这样就会导致min_dist到2*min_dist之间没有几个匹配。
    // 所以，在2*min_dist小于30的时候，就取30当上限值，小于30即可，不用2*min_dist这个值了
    std::vector<DMatch> good_matches;
    for (int j = 0; j < descriptors_1.rows; ++j) {
        if (matches[j].distance <= max(2 * min_dist, 30.0)) {
            good_matches.push_back(matches[j]);
        }

    }

    //第六步：绘制匹配结果

//    Mat img_match;//所有匹配点图
    //这里看一下drawMatches()原型参数，简单用法就是：图1，图1关键点，图2，图2关键点，匹配数组，承接图像，后面的有默认值
    /*
        CV_EXPORTS_W void drawMatches( InputArray img1,
                                    const std::vector<KeyPoint>& keypoints1,
                                    InputArray img2,
                                    const std::vector<KeyPoint>& keypoints2,
                                    const std::vector<DMatch>& matches1to2,
                                    InputOutputArray outImg,
                                    const Scalar& matchColor=Scalar::all(-1),
                                    const Scalar& singlePointColor=Scalar::all(-1),
                                    const std::vector<char>& matchesMask=std::vector<char>(),
                                    int flags=DrawMatchesFlags::DEFAULT );
    */
//    drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_match);
//    imshow("所有匹配点对", img_match);
//
//    Mat img_goodmatch;//筛选后的匹配点图
//    drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_goodmatch);
//    imshow("筛选后的匹配点对", img_goodmatch);

    //-- Localize the object from img_1 in img_2
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for (int i = 0; i < good_matches.size(); i++) {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints_1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints_2[good_matches[i].trainIdx].pt);
    }

    if (obj.size() <= 3) {
        printf("no enough point");
        jintArray array = env->NewIntArray(2);
        jint *elems = env->GetIntArrayElements(array, NULL);
        elems[0] = 0;
        elems[1] = 0;
        env->ReleaseIntArrayElements(array, elems, 0); // 取值 零(0) 时，更新数组并释放所有元素; http://blog.sina.com.cn/s/blog_4c451e0e0101339r.html
        return array;
    }

    Mat H = findHomography(obj, scene, CV_RANSAC);

    vector<Point2f> points, points_trans;
    points.push_back(highest_point1);
    perspectiveTransform(points, points_trans, H);

    point1_trans = points_trans[0];


    jintArray array = env->NewIntArray(2);
    jint *elems = env->GetIntArrayElements(array, NULL);
    elems[0] = point1_trans.x;
    elems[1] = point1_trans.y;
    env->ReleaseIntArrayElements(array, elems, 0); // 取值 零(0) 时，更新数组并释放所有元素; http://blog.sina.com.cn/s/blog_4c451e0e0101339r.html
    return array;

//    circle(img, points_trans[0], 5, Scalar(0, 0, 255), -1);
//    highest_point2;
//    circle(img, highest_point2,  5, Scalar(0, 0, 255), -1);

}



extern "C"
JNIEXPORT jintArray JNICALL
Java_com_jiangdg_usbcamera_view_USBCameraActivity_crop(JNIEnv *env, jclass type,
                                                       jlong jrgba, jint width, jint height) {

    Mat &img = *(Mat *) jrgba;


    Mat crop_image = img(Rect(0, point1_trans.y, width, highest_point2.y - point1_trans.y));
    // https://www.jianshu.com/p/5f6c728981b7
    int fan1 = crop_image.channels(); //3
    int fan2 = crop_image.type(); // 16 CV_8UC3 https://blog.csdn.net/pekingFloater/article/details/82688327
    int fan3 = crop_image.elemSize(); // 3
    int fan4 = crop_image.elemSize1(); // 1
    int fan5 = crop_image.depth(); // 0

//    cvtColor(crop_image, crop_image, COLOR_RGB2RGBA);
    cvtColor(crop_image, crop_image, COLOR_RGB2BGRA); // 需要转化为BGR否则颜色会异常

    int fan6 = crop_image.channels(); //3
    int fan7 = crop_image.type(); // 16 CV_8UC3 https://blog.csdn.net/pekingFloater/article/details/82688327
    int fan8 = crop_image.elemSize(); // 3
    int fan9 = crop_image.elemSize1(); // 1
    int fan10 = crop_image.depth(); // 0


    int size = width * (highest_point2.y - point1_trans.y);
//    int length = (int) (crop_image.total() * crop_image.elemSize());//total返回数组元素的总数  elemSize该方法返回以字节为单位的矩阵元素大小。例如，如果矩阵类型是 CV_16SC3，该方法返回3*sizeof(short)或 6

//
//    Mat out(h,w,CV_8UC4);
//    bilateralFilter(img,out,25,25*2,25/2);

//    jint* out = (jint *) reimg.data;
//    jint* re =NULL;
//    for (int i = 0; i <w*h ; ++i) {
//        re[4*i+0]=out[3*i+0];
//        re[4*i+1]=out[3*i+1];
//        re[4*i+2]=out[3*i+2];
//        re[4*i+2]=outimg.at(4).ptr(0)[i];
//    }

    jint *re = (jint *) crop_image.data;
//    int size = w * h;

    jintArray array = env->NewIntArray(size + 4);

    jint *elems = env->GetIntArrayElements(array, NULL);

    elems[0] = width;
    elems[1] = (highest_point2.y - point1_trans.y);
    elems[2] = point1_trans.x;
    elems[3] = highest_point2.x;

    for (int i = 4; i < size + 4; i++) {
        elems[i] = re[i - 4];
    }
    env->ReleaseIntArrayElements(array, elems,
                                 0); // 取值 零(0) 时，更新数组并释放所有元素; http://blog.sina.com.cn/s/blog_4c451e0e0101339r.html
//    作者：会飞的大象_
//    链接：https://www.jianshu.com/p/c4519aeea026
//    来源：简书
//    著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

//    env->SetIntArrayRegion(array, 0, 1,(jint*)width);
//    env->SetIntArrayRegion(array, 1, 1,(jint*)(highest_point2.y - point1_trans.y));
//
//
//    env->SetIntArrayRegion(array, 2, size, re);
//    env->ReleaseIntArrayElements(pixels_, pixels, 0);


    return array;


//    return crop_image;
}

}
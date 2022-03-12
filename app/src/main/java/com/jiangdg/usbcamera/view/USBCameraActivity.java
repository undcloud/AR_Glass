package com.jiangdg.usbcamera.view;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.usb.UsbDevice;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;

import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import com.jiangdg.usbcamera.R;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.jiangdg.usbcamera.UVCCameraHelper;
import com.jiangdg.usbcamera.application.MyApplication;
import com.jiangdg.usbcamera.utils.FileUtils;
import com.serenegiant.usb.CameraDialog;
import com.serenegiant.usb.Size;
import com.serenegiant.usb.USBMonitor;
import com.serenegiant.usb.common.AbstractUVCCameraHandler;
import com.serenegiant.usb.encoder.RecordParams;
import com.serenegiant.usb.widget.CameraViewInterface;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;



import android.Manifest;
import android.content.ClipboardManager;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
//import android.support.annotation.Nullable;
//import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;
import com.tbruyelle.rxpermissions2.RxPermissions;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import static org.opencv.imgproc.Imgproc.cvtColor;




import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.FormBody;
import okhttp3.MediaType;
import okhttp3.MultipartBody;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;


/**
 * UVCCamera use demo
 * <p>
 * Created by jiangdongguo on 2017/9/30.
 */




public class USBCameraActivity extends AppCompatActivity implements CameraDialog.CameraDialogParent, CameraViewInterface.Callback {

    private JavaCameraView cameraView;
    private Mat rgba;
    static double[] x1;
    static double[] x2;
    static double[] y1;
    static double[] y2;
    boolean bool_grid_create = false;
    boolean button1_bool = false;
    //    private String url = "http://192.168.124.10:8000/pics/";//替换成自己的服务器地址
//    private String url = "http://223.21.244.199:9023/pics/";//替换成自己的服务器地址
    private String url = "http://3z402o0862.wicp.vip/pics/";

    static {
        System.loadLibrary("native-lib");
    }


    public static native void nativeRgba(long jrgba, int width, int height);

    public static native void gridShow(long jrgba, int width, int height,double[] x1, double[] y1, double[] x2, double[] y2);
    public static native void gridShow2(long jrgba, int width, int height);
    public static native void handHistogram(long jrgba, int width, int height,double[] x1, double[] y1, double[] x2, double[] y2);
    public static native void handHistogram2(long jrgba, int width, int height);

    public static native void getNail(long jrgba, int width, int height);
    public static native int[] getNail2(long jrgba, int width, int height);

    public static native void preprocess(long jrgba, int width, int height);

    public static native void nail1(long jrgba, int width, int height);
    public static native void nail2(long jrgba, int width, int height);
    public static native int[] nail2_(long jrgba, int width, int height);


    public static native int[] crop(long jrgba, int width, int height);




    private static final String TAG = "Debug";
    @BindView(R.id.camera_view) // https://blog.csdn.net/xiayu54/article/details/90742737  代替：findviewbyid方法  https://www.jianshu.com/p/6492b66be33d
    public View mTextureView;
    @BindView(R.id.toolbar)
    public Toolbar mToolbar;
    @BindView(R.id.seekbar_brightness)
    public SeekBar mSeekBrightness; // 滑动条 fan
    @BindView(R.id.seekbar_contrast)
    public SeekBar mSeekContrast;
    @BindView(R.id.switch_rec_voice)
    public Switch mSwitchVoice;

    private UVCCameraHelper mCameraHelper;
    private CameraViewInterface mUVCCameraView;
    private AlertDialog mDialog;

    private boolean isRequest;
    private boolean isPreview;

    private UVCCameraHelper.OnMyDevConnectListener listener = new UVCCameraHelper.OnMyDevConnectListener() {

        @Override
        public void onAttachDev(UsbDevice device) {
            // request open permission
            if (!isRequest) {
                isRequest = true;
                if (mCameraHelper != null) {
                    mCameraHelper.requestPermission(0);
                }
            }
        }

        @Override
        public void onDettachDev(UsbDevice device) {
            // close camera
            if (isRequest) {
                isRequest = false;
                mCameraHelper.closeCamera();
                showShortMsg(device.getDeviceName() + " is out");
            }
        }

        @Override
        public void onConnectDev(UsbDevice device, boolean isConnected) {
            if (!isConnected) {
                showShortMsg("fail to connect,please check resolution params");
                isPreview = false;
            } else {
                isPreview = true;
                showShortMsg("connecting");
                // initialize seekbar
                // need to wait UVCCamera initialize over
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            Thread.sleep(2500);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        Looper.prepare(); // 调用Looper.prepare()来给线程创建一个消息循环，调用Looper.loop()来使消息循环起作用 https://www.cnblogs.com/l2rf/p/6055218.html
                        if(mCameraHelper != null && mCameraHelper.isCameraOpened()) {
                            mSeekBrightness.setProgress(mCameraHelper.getModelValue(UVCCameraHelper.MODE_BRIGHTNESS));// 手动调整亮度 fan
                            mSeekContrast.setProgress(mCameraHelper.getModelValue(UVCCameraHelper.MODE_CONTRAST));// 手动调整对比对 fan
                        }
                        Looper.loop();
                    }
                }).start();
            }
        }

        @Override
        public void onDisConnectDev(UsbDevice device) {
            showShortMsg("disconnecting");
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_usbcamera);
        ButterKnife.bind(this);
        initView();

        // step.1 initialize UVCCameraHelper
        mUVCCameraView = (CameraViewInterface) mTextureView;
        mUVCCameraView.setCallback(this);
        mCameraHelper = UVCCameraHelper.getInstance();
        mCameraHelper.setDefaultFrameFormat(UVCCameraHelper.FRAME_FORMAT_MJPEG);
        mCameraHelper.initUSBMonitor(this, mUVCCameraView, listener);

        final ImageView surface = (ImageView)findViewById(R.id.surface);
        final ImageView surface_debug = (ImageView)findViewById(R.id.surface_debug);

        TextView tv = (TextView)findViewById(R.id.textshow);

        // 就是这里
        mCameraHelper.setOnPreviewFrameListener(new AbstractUVCCameraHandler.OnPreViewResultListener() {

            int printNum = 0;

            @Override
            public void onPreviewResult(byte[] nv21Yuv) {
                printNum++;
                if (printNum == 50) {
                    printNum = 0;
//                    Bitmap b = Bitmap.createBitmap(surface.getWidth(),  surface.getHeight(), Bitmap.Config.ARGB_8888);
                    //  还不清楚为什么不能直接用surface.getHeight，可能高度发生了变化，还是得根据相机的长宽比来 fan
                    Bitmap b = Bitmap.createBitmap(surface.getWidth(),  (int)(surface.getWidth()/mUVCCameraView.getAspectRatio()), Bitmap.Config.ARGB_8888);
                    Log.d("surface_size", String.valueOf(surface.getWidth()) + "," + String.valueOf((int)(surface.getWidth()/mUVCCameraView.getAspectRatio())) + String.valueOf(surface.getHeight()));
                    Canvas canvas = new Canvas(b);
                    Paint p = new Paint();
                    p.setColor(Color.RED);// 设置红色
                    Paint p_blue = new Paint();
                    p_blue.setColor(Color.BLUE);// 设置红色

                    Log.d(TAG, "onPreviewResult: " + nv21Yuv.length + "摄像头预览");

                    Log.d(TAG, "onPreviewResult: " + nv21Yuv.length);// 460800
                    //Bitmap bitmap = BitmapFactory.decodeByteArray(nv21Yuv, 0, nv21Yuv.length);

                    YuvImage yuvImage = new android.graphics.YuvImage(nv21Yuv, ImageFormat.NV21, mCameraHelper.getPreviewWidth(), mCameraHelper.getPreviewHeight(), null);

                    ByteArrayOutputStream os = new ByteArrayOutputStream();
                    yuvImage.compressToJpeg(new Rect(0, 0, mCameraHelper.getPreviewWidth(), mCameraHelper.getPreviewHeight()), 100, os); // https://www.cnblogs.com/Ayinger/p/11006354.html
                    byte[] jpegByteArray = os.toByteArray();
                    // 原图
                    Bitmap bitmap = BitmapFactory.decodeByteArray(jpegByteArray, 0, jpegByteArray.length);
//                    final FritzVisionImage fritzVisionImage = FritzVisionImage.fromBitmap(bitmap);
                    Log.d(TAG, "mCameraHelper.getPreviewWidth(): " + mCameraHelper.getPreviewWidth() + "mCameraHelper.getPreviewHeight():" + mCameraHelper.getPreviewHeight()); // 默认640,480  最大1280,720
                    Log.d("config", bitmap.getConfig().toString()); // ARGB_8888

//                    surface_debug.setImageBitmap(bitmap);

                    Mat original_mat = new Mat(bitmap.getWidth(), bitmap.getHeight(), CvType.CV_8UC4);
                    Utils.bitmapToMat(bitmap, original_mat); // https://blog.csdn.net/hfut_why/article/details/84706098
//                    cvtColor(original_mat,original_mat,Imgproc.COLOR_RGBA2GRAY);
                    cvtColor(original_mat,original_mat,Imgproc.COLOR_RGBA2RGB);
                    long addr = original_mat.getNativeObjAddr();

                    // 点击创建颜色直方图 fan
                    findViewById(R.id.skinButton).setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            bool_grid_create = true;
//                showShortMsg("范超hello");
//                handHistogram(addr,rgba.width(),rgba.height(), CameraOpenCVActivity.this.x1, CameraOpenCVActivity.this.y1, CameraOpenCVActivity.this.x2, CameraOpenCVActivity.this.y2);
                            handHistogram2(addr, original_mat.width(), original_mat.height());
//                            showShortMsg("hello world!");
                        }
                    });

                    findViewById(R.id.nailButton1).setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            nail1(addr, original_mat.width(), original_mat.height());
//                            showShortMsg("nail1 get");
                            button1_bool = true;
                        }
                    });

                    findViewById(R.id.nailButton2).setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            // 确定第一个点 和 第二个点， 根据这两个点 截取图片，发送POST请求OCR分析，将结果复制到剪切板
//                nail2(addr, rgba.width(), rgba.height());
                            int[] datas = crop(addr, original_mat.width(),original_mat.height());
//                            Toast.makeText(USBCameraActivity.this,"nail2 get",Toast.LENGTH_LONG).show();
                            int width = datas[0];
                            int height = datas[1];
                            String point1_x = Integer.toString(datas[2]);
                            String point2_x = Integer.toString(datas[3]);

                            int[] cropPixels = new int[width*height];
                            for(int i =0; i<=(width*height)-1; i++){
                                cropPixels[i] = datas[i+4];
                            }

//                Mat rgba_temp = new Mat(rgba.width(), rgba.height(), CvType.CV_8UC4);
//                cvtColor(rgba,rgba_temp, Imgproc.COLOR_RGB2RGBA );
//                Bitmap temp = Bitmap.createBitmap(rgba.width(), rgba.height(), Bitmap.Config.ARGB_8888);
//                Utils.matToBitmap(rgba_temp,temp);

                            Bitmap cropBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                            cropBitmap.setPixels(cropPixels,0,width,0,0,width,height); // https://blog.csdn.net/QPC908694753/article/details/79080418

                            surface_debug.setImageBitmap(cropBitmap); // 在下面显示剪切的图片

                            // bitmap->byte[]
                            ByteArrayOutputStream baos = new ByteArrayOutputStream();
                            cropBitmap.compress(Bitmap.CompressFormat.JPEG, 30, baos);
                            byte[] cropDatas = baos.toByteArray();


                            RequestBody fileBody = RequestBody.create(MediaType.parse("image/jpg"), cropDatas); //https://www.jianshu.com/p/ef9bd1faeb08
//        MultipartBody multipartBody = new MultipartBody.Builder()
//                .setType(MultipartBody.FORM)
//                .addFormDataPart("image", "IMG_20200831_074429.jpg", fileBody)
//                .build();
                            MultipartBody multipartBody = new MultipartBody.Builder()//https://www.jianshu.com/p/ef9bd1faeb08
                                    .setType(MultipartBody.FORM)
                                    .addFormDataPart("key", "abc")
                                    .addFormDataPart("file", "test", fileBody)
                                    .addFormDataPart("point1_x",  point1_x)
                                    .addFormDataPart("point2_x",  point2_x)
                                    .build();

                            Request request = new Request.Builder()
                                    .url(url)
                                    .post(multipartBody)
                                    .build();
                            OkHttpClient client = new OkHttpClient.Builder()
                                    .readTimeout(800, TimeUnit.SECONDS)
                                    .connectTimeout(800, TimeUnit.SECONDS)
                                    .writeTimeout(800,TimeUnit.SECONDS)
                                    .build();


//        OkHttpClient client = new OkHttpClient();
//        FormBody.Builder formBuilder = new FormBody.Builder();
//        formBuilder.add("demo", str_value);
//
//        Request request = new Request.Builder().url(url).post(formBuilder.build()).build();


                            Call call = client.newCall(request);
                            call.enqueue(new Callback() {
                                @Override
                                public void onFailure(Call call, IOException e) {
                                    System.out.println(e.getMessage());
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            runOnUiThread(new Runnable() {
                                                @Override
                                                public void run() {

                                                    Toast.makeText(USBCameraActivity.this, "服务器错误", Toast.LENGTH_SHORT).show();
                                                    tv.setText("服务器错误");
                                                }
                                            });
                                        }
                                    });
                                }

                                @Override
                                public void onResponse(Call call, final Response response) throws IOException {
                                    final String res = response.body().string();
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            if (res.equals("0")) {
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        Toast.makeText(USBCameraActivity.this, "失败", Toast.LENGTH_SHORT).show();
                                                        tv.setText("失败");

                                                    }
                                                });
                                            } else {
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        ClipboardManager cm = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
                                                        cm.setText(res);
                                                        Toast.makeText(USBCameraActivity.this, "成功"+res, Toast.LENGTH_SHORT).show();
                                                        tv.setText("成功： "+res);
                                                    }
                                                });

                                            }

                                        }
                                    });
                                }
                            });


                        }
                    });

                    // 辅助画图 放在处理后面，以免影响处理过程
                    if (bool_grid_create == false) {
                        //对一帧图像进行处理
                    //        nativeRgba(addr,rgba.width(),rgba.height());
                    //            gridShow(addr, rgba.width(), rgba.height(), this.x1, this.y1, this.x2, this.y2);
//                        showShortMsg("start debugging");
                        Paint p_grid = new Paint();
                        p_grid.setColor(Color.RED);// 设置红色
                        p_grid.setStyle(Paint.Style.STROKE); // 设置空心

                        float grid_width =  ((float)50/bitmap.getWidth()) * b.getWidth() ;
                        float grid_height = ((float)200/bitmap.getHeight()) * b.getHeight() ;// b.getHeight和surface.getHeight值不一样.

                        canvas.drawRect(b.getWidth()/2 - grid_width/2,b.getHeight()/2 - grid_height/2,b.getWidth()/2 + grid_width/2,b.getHeight()/2 + grid_height/2, p_grid);
//                        gridShow2(addr, original_mat.width(), original_mat.height());

                        //                        canvas.drawRect(50,50,100,100, p_grid);
//                        Log.d("grid_width", Float.toString(grid_width)); //135
//                        Log.d("grid_height", Float.toString(grid_height)); //455
//                        Log.d("b.getWidth()", Float.toString(b.getWidth())); //1080
//                        Log.d("b.getHeight()", Float.toString(b.getHeight())); //809
//                        Log.d("drawRect", Float.toString(b.getWidth()/2 - grid_width/2) + ',' + Float.toString(b.getHeight()/2 - grid_height/2) + ',' + Float.toString(b.getWidth()/2 + grid_width/2) + ',' +Float.toString(b.getHeight()/2 + grid_height/2));


//                        int grid_w = 80;
//                        int grid_h = 270;
//                        rectangle(img, Point(width/2-grid_w/2, height/2-grid_h/2), Point(width/2+grid_w/2, height/2+grid_h/2), Scalar(1));

//                        handHistogram2(addr, original_mat.width(), original_mat.height());
//                        showShortMsg("no bug in handHistogram2");


// 延时一秒没有问题
//                        new Handler().postDelayed(new Runnable() {
//                            @Override
//                            public void run() {
//
//                                /**
//                                 * 延时执行的代码
//                                 */
//
//                            }
//                        },1000); // 延时1秒
//                        showShortMsg("no delay bug here");



                        //得到一帧灰度图
                    //        rgba = inputFrame.gray();
                    }else{
                        // 肤色直方图匹配后，画出离中心最远的点
//                        showShortMsg("waiting !");
//                        showShortMsg("getNail done!");

                        int[] highest = getNail2(addr, original_mat.width(), original_mat.height());
                        int highest_x = highest[0];
                        int highest_y = highest[1];
                        canvas.drawCircle(((float)highest_x/original_mat.width())*b.getWidth(), ((float)highest_y/original_mat.height())*b.getHeight(), 10, p);
//                        getNail(addr, original_mat.width(), original_mat.height()); // 画出当前最高点

                        if (button1_bool==true) {
//                            nail2(addr, original_mat.width(), original_mat.height()); // 画出转变坐标后的第一个点
//                            showShortMsg("nail12 getting");
                            int[] nail1_trans = nail2_(addr, original_mat.width(), original_mat.height());
                            int nail_trans_x = nail1_trans[0];
                            int nail_trans_y = nail1_trans[1];
                            canvas.drawCircle(((float)nail_trans_x/original_mat.width())*b.getWidth(), ((float)nail_trans_y/original_mat.height())*b.getHeight(), 10,  p_blue);
                        }
                    }



                    // 输出surface_debug   fan
//                    Bitmap bitmap_debug = Bitmap.createBitmap(original_mat.width(), original_mat.height(), Bitmap.Config.ARGB_8888);
//                    Utils.matToBitmap(original_mat, bitmap_debug,true);//添加透明度
//                    surface_debug.setImageBitmap(bitmap_debug);

                    // 输出imageview
//                    canvas.drawCircle(0, 0, 10, p);// 小圆 从这里可以发现，ImageView相对比CameraView会往下偏移大概100多个像素  fan
//                    canvas.drawCircle(60, 20, 10, p);// 小圆
//                    canvas.drawCircle(120, 40, 10, p);// 小圆
//                    canvas.drawCircle(180, 60, 10, p);// 小圆
                    surface.setImageBitmap(b);
                }
                else
                {
//                    surface.setImageDrawable(null); //是否让canvas停留
                }
            }
        });
    }

    private void initView() {
        setSupportActionBar(mToolbar);

        mSeekBrightness.setMax(100);
        mSeekBrightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(mCameraHelper != null && mCameraHelper.isCameraOpened()) {
                    mCameraHelper.setModelValue(UVCCameraHelper.MODE_BRIGHTNESS,progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        mSeekContrast.setMax(100);
        mSeekContrast.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(mCameraHelper != null && mCameraHelper.isCameraOpened()) {
                    mCameraHelper.setModelValue(UVCCameraHelper.MODE_CONTRAST,progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        // step.2 register USB event broadcast
        if (mCameraHelper != null) {
            mCameraHelper.registerUSB();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        // step.3 unregister USB event broadcast
        if (mCameraHelper != null) {
            mCameraHelper.unregisterUSB();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        /** 只有当Menu菜单首次被显示时会调用此方法 https://blog.csdn.net/a_moon721/article/details/5880260*/
        getMenuInflater().inflate(R.menu.main_toobar, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // 菜单的响应事件，根据ItemId辨别响应事件 https://blog.csdn.net/rayln/article/details/39152627
        // /** 当Menu有命令被选择时，会调用此方法 */  https://blog.csdn.net/a_moon721/article/details/5880260
        switch (item.getItemId()) {
            case R.id.menu_takepic:
                if (mCameraHelper == null || !mCameraHelper.isCameraOpened()) {
                    showShortMsg("sorry,camera open failed");
                    return super.onOptionsItemSelected(item);
                }
                String picPath = UVCCameraHelper.ROOT_PATH + MyApplication.DIRECTORY_NAME +"/images/"
                        + System.currentTimeMillis() + UVCCameraHelper.SUFFIX_JPEG;

                mCameraHelper.capturePicture(picPath, new AbstractUVCCameraHandler.OnCaptureListener() {
                    @Override
                    public void onCaptureResult(String path) {
                        if(TextUtils.isEmpty(path)) {
                            return;
                        }
                        new Handler(getMainLooper()).post(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(USBCameraActivity.this, "save path:"+path, Toast.LENGTH_SHORT).show();
                            }
                        });
                    }
                });

                break;
            case R.id.menu_recording:
                if (mCameraHelper == null || !mCameraHelper.isCameraOpened()) {
                    showShortMsg("sorry,camera open failed");
                    return super.onOptionsItemSelected(item);
                }
                if (!mCameraHelper.isPushing()) {
                    String videoPath = UVCCameraHelper.ROOT_PATH + MyApplication.DIRECTORY_NAME +"/videos/" + System.currentTimeMillis()
                            + UVCCameraHelper.SUFFIX_MP4;

//                    FileUtils.createfile(FileUtils.ROOT_PATH + "test666.h264");
                    // if you want to record,please create RecordParams like this
                    RecordParams params = new RecordParams();
                    params.setRecordPath(videoPath);
                    params.setRecordDuration(0);                        // auto divide saved,default 0 means not divided
                    params.setVoiceClose(mSwitchVoice.isChecked());    // is close voice

                    params.setSupportOverlay(true); // overlay only support armeabi-v7a & arm64-v8a
                    mCameraHelper.startPusher(params, new AbstractUVCCameraHandler.OnEncodeResultListener() {
                        @Override
                        public void onEncodeResult(byte[] data, int offset, int length, long timestamp, int type) {
                            // type = 1,h264 video stream
                            if (type == 1) {
                                FileUtils.putFileStream(data, offset, length);
                            }
                            // type = 0,aac audio stream
                            if(type == 0) {

                            }
                        }

                        @Override
                        public void onRecordResult(String videoPath) {
                            if(TextUtils.isEmpty(videoPath)) {
                                return;
                            }
                            new Handler(getMainLooper()).post(() -> Toast.makeText(USBCameraActivity.this, "save videoPath:"+videoPath, Toast.LENGTH_SHORT).show());
                        }
                    });
                    // if you only want to push stream,please call like this
                    // mCameraHelper.startPusher(listener);
                    showShortMsg("start record...");
                    mSwitchVoice.setEnabled(false);
                } else {
                    FileUtils.releaseFile();
                    mCameraHelper.stopPusher();
                    showShortMsg("stop record...");
                    mSwitchVoice.setEnabled(true);
                }
                break;
            case R.id.menu_resolution:
                if (mCameraHelper == null || !mCameraHelper.isCameraOpened()) {
                    showShortMsg("sorry,camera open failed");
                    return super.onOptionsItemSelected(item);
                }
                showResolutionListDialog();
                break;
            case R.id.menu_focus:
                if (mCameraHelper == null || !mCameraHelper.isCameraOpened()) {
                    showShortMsg("sorry,camera open failed");
                    return super.onOptionsItemSelected(item);
                }
                mCameraHelper.startCameraFoucs();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void showResolutionListDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(USBCameraActivity.this);
        View rootView = LayoutInflater.from(USBCameraActivity.this).inflate(R.layout.layout_dialog_list, null);
        ListView listView = (ListView) rootView.findViewById(R.id.listview_dialog);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(USBCameraActivity.this, android.R.layout.simple_list_item_1, getResolutionList());
        if (adapter != null) {
            listView.setAdapter(adapter);
        }
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
                if (mCameraHelper == null || !mCameraHelper.isCameraOpened())
                    return;
                final String resolution = (String) adapterView.getItemAtPosition(position);
                String[] tmp = resolution.split("x");
                if (tmp != null && tmp.length >= 2) {
                    int widht = Integer.valueOf(tmp[0]);
                    int height = Integer.valueOf(tmp[1]);
                    mCameraHelper.updateResolution(widht, height);
                }
                mDialog.dismiss();
            }
        });

        builder.setView(rootView);
        mDialog = builder.create();
        mDialog.show();
    }

    // example: {640x480,320x240,etc}
    private List<String> getResolutionList() {
        List<Size> list = mCameraHelper.getSupportedPreviewSizes();
        List<String> resolutions = null;
        if (list != null && list.size() != 0) {
            resolutions = new ArrayList<>();
            for (Size size : list) {
                if (size != null) {
                    resolutions.add(size.width + "x" + size.height);
                }
            }
        }
        return resolutions;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        FileUtils.releaseFile();
        // step.4 release uvc camera resources
        if (mCameraHelper != null) {
            mCameraHelper.release();
        }
    }

    private void showShortMsg(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    @Override
    public USBMonitor getUSBMonitor() {
        return mCameraHelper.getUSBMonitor();
    }

    @Override
    public void onDialogResult(boolean canceled) {
        if (canceled) {
            showShortMsg("取消操作");
        }
    }

    public boolean isCameraOpened() {
        return mCameraHelper.isCameraOpened();
    }

    @Override
    public void onSurfaceCreated(CameraViewInterface view, Surface surface) {
        if (!isPreview && mCameraHelper.isCameraOpened()) {
            mCameraHelper.startPreview(mUVCCameraView);
            isPreview = true;
        }
    }

    @Override
    public void onSurfaceChanged(CameraViewInterface view, Surface surface, int width, int height) {
        // 改变一个视频流窗口的大小 https://www.jianshu.com/p/39d4a49caef9
    }

    @Override
    public void onSurfaceDestroy(CameraViewInterface view, Surface surface) {
        if (isPreview && mCameraHelper.isCameraOpened()) {
            mCameraHelper.stopPreview();
            isPreview = false;
        }
    }
}

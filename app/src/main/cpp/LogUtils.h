#include "android/log.h"

#ifndef LOG_TAG
#define LOG_TAG "JNI"
#endif
#ifndef IS_DEBUG
#define IS_DEBUG true
#endif


#define LOG_NOOP (void) 0
//__FILE__ 输出文件名
//__LINE__ 输出行数
//__PRETTY_FUNCTION__  输出方法名
//__VA_ARGS__ 可变参数
//可以按需选取 %s %u %s 分别与之对应
#define LOG_PRINT(level,fmt,...) __android_log_print(level,LOG_TAG,"(%s:%u) %s: " fmt,__FILE__,__LINE__,__PRETTY_FUNCTION__,##__VA_ARGS__)
//通过IS_DEBUG来控制是否输出日志
#if IS_DEBUG
#define LOGI(fmt,...) LOG_PRINT(ANDROID_LOG_INFO,fmt,##__VA_ARGS__)
#else
#define LOGI(...) LOG_NOOP
#endif

#if IS_DEBUG
#define LOGW(fmt,...) LOG_PRINT(ANDROID_LOG_WARN,fmt ,##__VA_ARGS__)
#else
#define LOGW(...) LOG_NOOP
#endif

#if IS_DEBUG
#define LOGD(fmt,...) LOG_PRINT(ANDROID_LOG_DEBUG,fmt ,##__VA_ARGS__)
#else
#define LOGD(...) LOG_NOOP
#endif

#if IS_DEBUG
#define LOGE(fmt,...) LOG_PRINT(ANDROID_LOG_ERROR,fmt ,##__VA_ARGS__)
#else
#define LOGE(...) LOG_NOOP
#endif

#if IS_DEBUG
#define LOGF(fmt,...) LOG_PRINT(ANDROID_LOG_FATAL,fmt ,##__VA_ARGS__)
#else
#define LOGF(...) LOG_NOOP
#endif
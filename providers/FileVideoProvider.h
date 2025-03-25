#ifndef FILEVIDEOPROVIDER_H
#define FILEVIDEOPROVIDER_H

#include "VideoProvider.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <string>
#include <map>

/**
 * @class FileVideoProvider
 * @brief 从文件或网络流中提供视频帧的类，继承自VideoProvider基类。
 * 
 * 该类使用FFmpeg库来打开、解码视频文件或网络流（如RTSP、RTMP），
 * 并将解码后的视频帧放入队列中供后续处理。
 */
class FileVideoProvider : public VideoProvider
{
private:
    /**
     * @brief 视频源的URL，可以是本地文件路径或RTSP、RTMP网络地址。
     */
    std::string url;
    /**
     * @brief FFmpeg格式上下文，用于处理视频文件或流的格式信息。
     */
    AVFormatContext *formatCtx = nullptr;
    /**
     * @brief FFmpeg编解码器上下文，用于处理视频帧的编解码操作。
     */
    AVCodecContext *codecCtx = nullptr;
    /**
     * @brief FFmpeg编解码器指针，指向具体的视频编解码器。
     */
    const AVCodec *codec = nullptr;
    /**
     * @brief 视频流在格式上下文中的索引，用于标识视频流。
     */
    int videoStreamIndex = -1;
    /**
     * @brief FFmpeg图像缩放上下文，用于在不同像素格式和尺寸之间转换视频帧。
     */
    SwsContext *swsCtx = nullptr;
    /**
     * @brief 硬解码器映射表，将软解码名称映射到对应的硬解码器的名字。
     * 
     * 静态常量映射表，用于根据特定的键查找对应的解码器名称。
     */
    static const std::map<std::string, std::string> decoder_map;

public:
    /**
     * @brief 构造函数，初始化文件视频提供者。
     * 
     * @param url 视频源的URL，可以是本地文件路径或RTSP、RTMP网络地址，默认为nullptr。
     */
    FileVideoProvider(const char *url = nullptr);
    /**
     * @brief 初始化视频源，打开文件或网络流，查找解码器并初始化编解码上下文。
     * 
     * @return bool 初始化成功返回true，失败返回false。
     */
    bool init();
    /**
     * @brief 停止视频提供线程，释放相关资源。
     */
    void stop();
    /**
     * @brief 线程执行的主要方法，使用FFmpeg不断解码视频流并获取时间戳，将解码后的帧放入队列。
     */
    void run();
};

#endif // FILEVIDEOPROVIDER_H

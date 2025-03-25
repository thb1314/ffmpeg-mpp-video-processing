#ifndef XMEDIAENCODE_H
#define XMEDIAENCODE_H

#include <string>

struct AVFrame;
struct AVPacket;
struct AVCodecContext;


/**
 * @class XMediaEncode
 * @brief 媒体编码的抽象基类，提供媒体编码的通用接口和参数配置
 * 
 * 该类定义了媒体编码所需的输入输出参数，以及一系列纯虚函数，
 * 用于实现像素格式转换、视频编码等功能。派生类需要实现这些纯虚函数以完成具体的编码操作。
 */
class XMediaEncode
{
public:
    /// 输入参数
    int inWidth = 1280;  ///< 输入视频帧的宽度，默认为1280像素
    int inHeight = 720;  ///< 输入视频帧的高度，默认为720像素
    int inPixSize = 3;   ///< 输入像素的大小，默认为3字节（如RGB格式）

    /// 输出参数
    int outWidth = inWidth;  ///< 输出视频帧的宽度，默认为输入宽度
    int outHeight = inHeight; ///< 输出视频帧的高度，默认为输入高度
    int bitrate = 4000000; ///< 压缩后每秒视频的比特位大小，默认为4000000bps（约500kB/s）
    int fps = 25;  ///< 输出视频的帧率，默认为25帧每秒

    /**
     * @brief 工厂方法，获取XMediaEncode实例
     * 
     * @param index 实例的索引，默认为0
     * @return XMediaEncode* 返回XMediaEncode实例的指针
     */
    static XMediaEncode *getInstance(unsigned char index = 0);

    /**
     * @brief 初始化像素格式转换的上下文
     * 
     * 该方法用于初始化像素格式转换所需的上下文，以便后续进行像素格式转换操作。
     * @return bool 初始化成功返回true，失败返回false
     */
    virtual bool initScale() = 0;

    /**
     * @brief 将RGB格式的视频数据转换为YUV格式
     * 
     * 该方法将输入的RGB格式视频数据转换为YUV格式的AVFrame对象。
     * 返回的AVFrame对象无需调用者清理。
     * @param rgb 输入的RGB格式视频数据指针
     * @return AVFrame* 转换后的YUV格式AVFrame对象指针，失败时返回nullptr
     */
    virtual AVFrame *rgb2yuv(char *rgb) = 0;

    /**
     * @brief 初始化视频编码器
     * 
     * 该方法用于初始化视频编码器，为后续的视频编码操作做准备。
     * @return bool 初始化成功返回true，失败返回false
     */
    virtual bool initVideoCodec() = 0;

    /**
     * @brief 对视频帧进行编码
     * 
     * 该方法对输入的AVFrame对象进行编码，生成编码后的AVPacket对象。
     * @param frame 输入的待编码视频帧
     * @param pts 视频帧的显示时间戳
     * @return AVPacket* 编码后的AVPacket对象指针，失败时返回nullptr
     */
    virtual AVPacket *encodeVideo(AVFrame *frame, int64_t pts) = 0;

    /**
     * @brief 设置最后一次错误信息
     * 
     * 该方法用于设置编码过程中发生的最后一次错误信息。
     * @param error 错误信息字符串
     */
    virtual void setLastError(const std::string &) = 0;

    /**
     * @brief 获取最后一次错误信息
     * 
     * 该方法用于获取编码过程中发生的最后一次错误信息。
     * @return const std::string& 最后一次错误信息的常量引用
     */
    virtual const std::string &getLastError(void) = 0;

    /**
     * @brief 析构函数
     * 
     * 虚析构函数，确保在删除指向派生类对象的基类指针时能正确释放资源。
     */
    virtual ~XMediaEncode() = default;

    /**
     * @brief 关闭编码器并释放资源
     * 
     * 该方法用于关闭编码器，释放相关的资源。
     */
    virtual void close() = 0;

    AVCodecContext *vc = 0; // 编码器上下文，用于存储视频编码器的相关参数

protected:
    /**
     * @brief 默认构造函数
     * 
     * 受保护的默认构造函数，防止外部直接实例化该抽象基类。
     */
    XMediaEncode() = default;

    /**
     * @brief 拷贝构造函数
     * 
     * 受保护的拷贝构造函数，防止外部直接拷贝该抽象基类的实例。
     * @param other 要拷贝的XMediaEncode对象引用
     */
    XMediaEncode(const XMediaEncode &other) = default;
};

#endif // XMEDIAENCODE_H

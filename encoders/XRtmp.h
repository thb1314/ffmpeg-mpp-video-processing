#ifndef XRTMP_H
#define XRTMP_H

#include <string>

class AVCodecContext;
class AVPacket;

/**
 * @class XRtmp
 * @brief 用于实现RTMP协议推流的抽象基类，提供了一系列操作RTMP推流的接口，同时也支持写入本地文件。
 * 
 * 该类定义了初始化封装器上下文、添加音视频流、发送封装头、推流等纯虚函数，
 * 派生类需要实现这些函数以完成具体的RTMP推流操作。
 */
class XRtmp
{
public:
    /**
     * @brief 工厂方法，用于获取XRtmp类的实例。
     * 
     * 通过该方法可以创建XRtmp类的派生类实例，用户可以根据需要指定实例的索引。
     * 
     * @param index 实例的索引，默认为0。
     * @return XRtmp* 返回XRtmp类实例的指针，失败时返回nullptr。
     */
    static XRtmp* getInstance(unsigned char index = 0);

    /**
     * @brief 初始化封装器上下文。
     * 
     * 该方法用于初始化RTMP推流活文件写入所需的封装器上下文，连接到指定的RTMP服务器URL或本地文件。
     * 
     * @param url RTMP服务器的URL地址或者本地文件地址。
     * @return bool 初始化成功返回true，失败返回false。
     */
    virtual bool init(const char* url) = 0;

    /**
     * @brief 向封装器中添加视频或音频流。
     * 
     * 该方法根据传入的编解码器上下文信息，向封装器中添加对应的音视频流，
     * 并返回该流在封装器中的索引。
     * 
     * @param c 指向AVCodecContext的指针，包含编解码器的上下文信息。
     * @return int 成功添加流后返回对应的索引，失败返回-1。
     */
    virtual int addStream(const AVCodecContext* c) = 0;


    /**
     * @brief 打开RTMP网络IO或者文件IO，并发送封装头信息。
     * 
     * 该方法用于建立与RTMP服务器的网络连接，并发送封装头信息，
     * 为后续的音视频帧推流做准备。
     * 
     * @return bool 操作成功返回true，失败返回false。
     */
    virtual bool sendHead() = 0;


    /**
     * @brief 向RTMP服务器推流音视频帧。
     * 
     * 该方法将编码后的音视频数据包推送到RTMP服务器，需要指定数据包和对应的流索引。
     * 
     * @param pkt 指向AVPacket的指针，包含编码后的音视频数据包。
     * @param index 流的索引，用于标识音视频流。
     * @return bool 推流成功返回true，失败返回false。
     */
    virtual bool sendFrame(AVPacket* pkt, int index) = 0;

    /**
     * @brief 设置最后一次发生的错误信息。
     * 
     * 该方法用于记录推流过程中发生的错误信息，方便后续排查问题。
     * 
     * @param error 错误信息字符串。
     */
    virtual void setLastError(const std::string&) = 0;

    /**
     * @brief 获取最后一次发生的错误信息。
     * 
     * 该方法用于获取推流过程中最后一次发生的错误信息。
     * 
     * @return const std::string& 最后一次错误信息的常量引用。
     */
    virtual const std::string& getLastError(void) = 0;

    /**
     * @brief 关闭RTMP连接并释放相关资源。
     * 
     * 该方法用于关闭与RTMP服务器的连接，释放之前分配的资源。
     */
    virtual void close() = 0;

    /**
     * @brief 析构函数。
     * 
     * 虚析构函数，确保在删除指向派生类对象的基类指针时能正确释放资源。
     */
    virtual ~XRtmp() = default;

protected:
    /**
     * @brief 默认构造函数。
     * 
     * 受保护的默认构造函数，防止外部直接实例化该抽象基类。
     */
    XRtmp() = default;

    /**
     * @brief 拷贝构造函数。
     * 
     * 受保护的拷贝构造函数，防止外部直接拷贝该抽象基类的实例。
     * 
     * @param other 要拷贝的XRtmp对象引用。
     */
    XRtmp(const XRtmp& other) = default;
};

#endif // XRTMP_H

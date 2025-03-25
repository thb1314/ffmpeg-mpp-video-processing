#ifndef VIDEOPROVIDER_H
#define VIDEOPROVIDER_H

#include "ThreadProvider.h"

/**
 * @brief 视频提供者的抽象基类，继承自 ThreadProvider 类
 * 
 * 该类定义了视频提供者的基本接口和属性，用于提供不同类型的视频源（如摄像头、文件）。
 * 派生类需要实现 init 方法来完成视频源的初始化工作。
 */
class VideoProvider : public ThreadProvider
{
public:
    /**
     * @brief 定义视频源的类型枚举
     * 
     * 枚举了两种常见的视频源类型：摄像头和文件。
     */
    enum VideoType
    {
        Camera = 0,  // 摄像头视频源
        File         // 文件视频源
    };

protected:
    int width = 0;         // 视频帧的宽度
    int height = 0;        // 视频帧的高度
    int fps = 0;           // 视频的帧率
    int frame_interval = 1;// 视频帧的间隔
    VideoType type = Camera; // 视频源的类型，默认为摄像头

public:
    /**
     * @brief 构造函数
     * 
     * @param type 视频源的类型，默认为摄像头
     */
    VideoProvider(VideoType type = Camera);

    /**
     * @brief 析构函数
     * 
     * 虚析构函数，确保在删除指向派生类对象的基类指针时能正确释放资源。
     */
    virtual ~VideoProvider();

    /**
     * @brief 初始化视频源
     * 
     * 纯虚函数，派生类需要实现该方法来完成视频源的初始化工作。
     * 
     * @return bool 初始化成功返回 true，失败返回 false
     */
    virtual bool init() = 0;

    /**
     * @brief 获取视频的帧率
     * 
     * @return int 视频的帧率
     */
    int getFps() const;

    /**
     * @brief 获取视频帧的高度
     * 
     * @return int 视频帧的高度
     */
    int getHeight() const;

    /**
     * @brief 获取视频帧的宽度
     * 
     * @return int 视频帧的宽度
     */
    int getWidth() const;

    /**
     * @brief 获取视频帧的间隔
     * 
     * @return int 视频帧的间隔
     */
    int getFrameInterval() const;

    /**
     * @brief 设置视频帧的间隔
     * 
     * @param interval 要设置的视频帧间隔
     * @return bool 设置成功返回 true，失败返回 false
     */
    bool setFrameInterval(int interval);
};

#endif // VIDEOPROVIDER_H

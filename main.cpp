#include <iostream>
#include <cstdint>
#include <ctime>
#include <memory>

#include "Utils.h"
#include "FileVideoProvider.h"
#include "XRtmp.h"
#include "XMediaEncode.h"


/**
 * @brief 将文件视频转换为FLV文件或推流到RTMP服务器
 * 
 * @return int 函数执行结果，0表示成功，负数表示失败
 */
int filevideo_to_flvfile()
{
    // av_register_all(); 
    // av_log_set_level(AV_LOG_DEBUG);
    // 创建一个智能指针管理视频提供者实例，使用指定的视频文件初始化
    std::unique_ptr<VideoProvider> video_provider(new FileVideoProvider("720p60hz.mp4"));
    // 设置视频帧间隔
    video_provider->setFrameInterval(2);
    // char outUrl[] = "0.mp4";
    // char outUrl[] = "rtsp://192.168.31.8:8554/live2";
    // 定义输出流的URL，这里是RTMP服务器地址
    char outUrl[] = "rtmp://192.168.31.8/live/stream1";
    // 标记是否为本地文件
    bool is_local_file = false;

    // 初始化并启动视频解析线程
    video_provider->init();
    video_provider->start();
    
    // 视频缩放配置
    // 获取视频编码实例
    XMediaEncode* xe = XMediaEncode::getInstance();
    // 设置编码器的帧率为视频提供者的帧率
    xe->fps = video_provider->getFps();
    
    // 设置编码器输入视频的高度
    xe->inHeight = video_provider->getHeight();
    // 设置编码器输出视频的高度为输入高度的一半
    xe->outHeight = video_provider->getHeight() / 2;
    // 设置编码器输入视频的宽度
    xe->inWidth = video_provider->getWidth();
    // 设置编码器输出视频的宽度为输入宽度的一半
    xe->outWidth = video_provider->getWidth() / 2;
    
    // 设置输入像素大小
    xe->inPixSize = 3;

    // 初始化视频缩放器
    if(!xe->initScale()) {
        std::cerr << "initScale error:"  << xe->getLastError() << std::endl;
        return -1;
    }
        
    // 初始化视频编码器
    if(!xe->initVideoCodec()) {
        std::cerr << "initVideoCodec error:" << xe->getLastError() << std::endl;
        return -1;
    }
    

    // 输出封装器和流配置
    // a 创建输出封装器上下文
    // 获取RTM实例
    XRtmp* xr = XRtmp::getInstance(0);
    // 初始化RTMP实例，传入输出URL（本地文件或者RTMP流）
    if(!xr->init(outUrl)){
        std::cerr << "init error:" << xr->getLastError() << std::endl;
        return -1;
    }
    // b 添加视频流
    int video_stream_index = -1;
    // 向RTMP实例添加视频流
    video_stream_index = xr->addStream(xe->vc);
    if(-1 == video_stream_index) {
        std::cerr << "addStream error:" << xr->getLastError() << std::endl;
        return -1;
    }
        
    // 打开rtmp 的网络输出IO
    // 写入封装头
    if(!xr->sendHead()){
        std::cerr << "sendHead error:" << xr->getLastError() << std::endl;
        return -1;
    }
    std::cout << "b1" << std::endl;
    int ret = 0;
    int64_t video_timestamp = 0;
    try
    {
        // 获取当前时间作为开始时间
        int64_t begintime = Utils::get_curtime();
        
        // 当视频提供者正在运行时，循环处理视频帧
        while(video_provider->isRunning())
        {
            // 获取视频帧数据包装器
            auto video_data_wraper = video_provider->top();
            // 获取视频帧数据大小
            int video_data_size = video_data_wraper.getByteSize();
            // 获取视频帧时间戳
            int64_t tmp_video_timestemp = video_data_wraper.getTimestamp();
  
            // 计算当前时间与开始时间的差值
            int64_t curTime = Utils::get_curtime() - begintime;
            // 判断是否需要推送视频帧
            bool is_push = is_local_file || tmp_video_timestemp <= curTime;
            if(0 != video_data_size && is_push)
            {
                // 移除当前视频帧
                video_provider->pop();
                // 获取视频帧数据指针
                char* video_data = (char*)video_data_wraper.getDataPtr();
                // 更新视频时间戳
                video_timestamp = video_data_wraper.getTimestamp();

                // 将RGB格式的视频数据转换为YUV格式
                AVFrame* yuv = xe->rgb2yuv(video_data);
                if(!yuv) {
                    std::cout << "rgb2yuv error" << std::endl;
                    continue;
                }
                // 对YUV格式的视频帧进行编码
                AVPacket* pkt = xe->encodeVideo(yuv, video_timestamp);
                if(!pkt) {
                    std::cout << "encode video error" << std::endl;
                    continue;
                }
                    
                // 推流
                std::cout << "video_timestamp:" << video_timestamp << std::endl;
                // 如果视频时间戳超过120秒，退出循环
                if(video_timestamp >= 120*1000000) {
                    break;
                }
                // 发送编码后的视频帧到RTMP服务器
                bool rer_val = xr->sendFrame(pkt, video_stream_index);
                if(rer_val)
                    std::cout << "@V@" << std::endl;
                
            }

        }
    }
    // 捕获异常并输出错误信息
    catch(std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    // 停止视频解析线程
    video_provider->stop();
    // 关闭视频编码器
    xe->close();
    // 关闭RTMP实例
    xr->close();
    return 0;
}



int main(int argc, char* argv[])
{
    
    std::cout << "begin--------" << std::endl;
    filevideo_to_flvfile();
    return 0;
}



#include "XMediaEncode.h"
#include "Utils.h"

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <iostream>
#include <map>
#include <string>

class CXMediaEncode : public XMediaEncode
{
private:
    static const std::map<std::string, std::string> encoder_map;
    std::string err_msg;

public:
    void setLastError(const std::string &buf)
    {
        err_msg = buf;
    }
    const std::string &getLastError()
    {
        return err_msg;
    }
    void close()
    {
        if (vsc)
        {
            sws_freeContext(vsc);
            vsc = NULL;
        }
        if (yuv)
        {
            av_frame_free(&yuv);
        }
        if (vc)
        {
            avcodec_free_context(&vc);
        }
        last_video_pts = 0;
        av_packet_unref(&vpack);

    }

    bool initVideoCodec()
    {
        // 初始化编码上下文
        //   a. 找到编码器
        const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);

        if (!codec)
        {
            this->setLastError("Can`t find h264 encoder!");
            return false;
        }
        bool use_hard_encoder = false;
        // 尝试寻找硬编码
        auto encoder_name = std::string(codec->name);
        auto it = encoder_map.find(encoder_name);
        if (it != encoder_map.end())
        {
            auto &try_encoder_name = it->second;
            const AVCodec *tmp_codec = avcodec_find_encoder_by_name(try_encoder_name.c_str());
            if (tmp_codec)
            {
                use_hard_encoder = true;
                codec = tmp_codec;
            }
        }
        std::cout << "encoder codec->name: " << codec->name << std::endl;
        //   b. 创建编码器上下文
        vc = avcodec_alloc_context3(codec);
        if (!vc)
        {
            this->setLastError("avcodec_alloc_context3 failed!");
            return false;
        }
        //   c. 配置编码器参数
        vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // 全局参数
        vc->codec_id = codec->id;
        vc->thread_count = use_hard_encoder ? 1 : Utils::core_count();

        // 压缩后每秒视频的最大bit位大小 200 kB
        vc->bit_rate = 200 * 1024 * 8;
        vc->width = outWidth;
        vc->height = outHeight;
        // 时间基准是us
        vc->time_base = {1, 1000000};
        vc->framerate = AVRational{this->fps, 1};

        // 画面组的大小，多少帧一个关键帧
        vc->gop_size = this->fps;
        vc->max_b_frames = 5;
        vc->pix_fmt = AV_PIX_FMT_YUV420P;

        //   d. 打开编码器上下文
        int ret = avcodec_open2(vc, 0, NULL);
        if (ret != 0)
        {
            char buf[1024] = {0};
            av_strerror(ret, buf, sizeof(buf) - 1);
            this->setLastError(buf);
            return false;
        }
        return true;
    }

    virtual AVPacket *encodeVideo(AVFrame *frame, int64_t pts)
    {
        av_packet_unref(&vpack);

        // h264编码
        while (pts == last_video_pts)
            pts += 1000;

        frame->pts = pts;
        last_video_pts = pts;

        // Supply a raw video or audio frame to the encoder.
        // Use avcodec_receive_packet() to retrieve buffered output packets.
        int ret = avcodec_send_frame(vc, frame);
        if (ret != 0)
            return NULL;
        // Read encoded data from the encoder.
        ret = avcodec_receive_packet(vc, &vpack);
        if (ret != 0 || vpack.size <= 0)
            return NULL;

        return &vpack;
    }

    bool initScale()
    {
        // 2. 初始化格式转换上下文
        vsc = sws_getCachedContext(vsc,
                                   inWidth, inHeight, AV_PIX_FMT_RGB24,     // 源宽、高、像素格式
                                   outWidth, outHeight, AV_PIX_FMT_YUV420P, // 目标宽、高、像素格式
                                   SWS_BICUBIC,                             // 尺寸变化使用算法
                                   0, 0, 0);
        if (!vsc)
        {
            this->setLastError("sws_getCachedContext failed!");
            return false;
        }

        // 3. 初始化输出的数据结构
        yuv = av_frame_alloc();
        yuv->format = AV_PIX_FMT_YUV420P;
        yuv->width = outWidth;
        yuv->height = outHeight;
        yuv->pts = 0;
        // 分配yuv空间
        int ret = av_frame_get_buffer(yuv, 32);
        if (ret != 0)
        {
            char buf[1024] = {0};
            av_strerror(ret, buf, sizeof(buf) - 1);
            this->setLastError(buf);
            return false;
        }
        return true;
    }

    AVFrame *rgb2yuv(char *rgb)
    {
        // 输入的数据结构
        uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
        // indata[0] bgr bgr bgr
        indata[0] = (uint8_t *)rgb;
        int insize[AV_NUM_DATA_POINTERS] = {0};
        // 一行（宽）数据的字节数
        insize[0] = inWidth * inPixSize;

        int h = sws_scale(vsc, indata, insize, 0, inHeight, // 源数据
                          yuv->data, yuv->linesize);
        if (h <= 0)
        {
            return NULL;
        }
        return yuv;
    }

private:
    int64_t last_video_pts = 0;
    SwsContext *vsc = NULL; // 像素格式转换上下文
    AVFrame *yuv = NULL;    // 输出的YUV
    AVPacket vpack = {0};
};

const std::map<std::string, std::string> CXMediaEncode::encoder_map = {
    {"h264", "h264_rkmpp"},
    {"libx264", "h264_rkmpp"},
    {"h265", "hevc_rkmpp"},
    {"henc", "hevc_rkmpp"},
    {"libx265", "hevc_rkmpp"},
};

XMediaEncode *XMediaEncode::getInstance(unsigned char index)
{
    static bool is_first = true;
    if (is_first)
    {
        // 注册所有的编解码器
        // avcodec_register_all();
        is_first = false;

        // 获取所有注册的编码器并输出它们的名称
        const AVCodec *codec = NULL;
        void *opaque = NULL;

        while ((codec = av_codec_iterate(&opaque)))
        {
            if (codec->type == AVMEDIA_TYPE_VIDEO)
            {
                // 输出编码器名称
                if (strstr(codec->name, "rk") != NULL)
                {
                    av_log(NULL, AV_LOG_INFO, "RK Related Video Encoder name: %s\n", codec->name);
                }
            }
        }
    }

    static CXMediaEncode cxm[255];
    return &cxm[index];
}

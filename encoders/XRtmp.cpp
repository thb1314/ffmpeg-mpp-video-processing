#include "XRtmp.h"

#include <iostream>
#include <string>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

class CXRtmp : public XRtmp
{
public:

    void close()
    {
        if(ic)
        {
            int ret;
            if(url.substr(0, 4) != "rtmp")
                av_write_trailer(ic);
            
            if(ic->pb)
                ret = avio_close(ic->pb);
            
            char buf[1024];
            av_strerror(ret, buf, sizeof(buf) - 1);
            this->setLastError(buf);
            std::cout << this->getLastError() << std::endl;
            avformat_free_context(ic);
            
        }
        
        vs = NULL;
        as = NULL;
        vc = NULL;
        ac = NULL;
        url.clear();
        std::cout << "10" << std::endl;
    }
    bool init(const char* url)
    {
        // 输出封装器和视频流配置
        // a 创建输出封装器上下文
        this->url = url;
        int ret = -1;
        if(this->url.substr(0, 4) == "rtmp")
            ret = avformat_alloc_output_context2(&ic, 0, "flv", url);
        else if(this->url.substr(0, 4) == "rtsp")
            ret = avformat_alloc_output_context2(&ic, NULL, "rtsp", url);
        else
            ret = avformat_alloc_output_context2(&ic, 0, 0, url);
        if(ret != 0)
        {
            char buf[1024] = { 0 };
            av_strerror(ret, buf, sizeof(buf) - 1);
            this->setLastError(buf);
            return false;
        }
        return true;
    }

    int addStream(const AVCodecContext* c)
    {
        if(!c)
            return false;

        //b 添加流
        AVStream* st = avformat_new_stream(ic, NULL);
        if(!st)
        {
            this->setLastError("avformat_new_stream failed");
            return -1;
        }
        st->codecpar->codec_tag = 0;
        // 从编码器复制参数
        avcodec_parameters_from_context(st->codecpar, c);
        av_dump_format(ic, 0, url.c_str(), 1);

        if(c->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vc = c;
            vs = st;
        }
        else if(c->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            ac = c;
            as = st;
        }

        return st->index;
    }

    bool sendHead()
    {
        // 打开rtmp 的网络输出IO
        AVDictionary* opts = NULL;
        av_dict_set(&opts, "rtsp_transport", "tcp", 0);
        // 网络延时时间
        av_dict_set(&opts, "max_delay", "500", 0);

        int ret = avio_open2(&ic->pb, url.c_str(), AVIO_FLAG_WRITE, NULL, &opts);
        av_dict_free(&opts);
        if(ret != 0)
        {   
            char buf[1024] = { 0 };
            av_strerror(ret, buf, sizeof(buf) - 1);
            av_log(NULL, AV_LOG_ERROR, "Could not open output URL '%s', error msg: %s\n", url.c_str(), buf);
            this->setLastError(buf);
            return false;
        }

        //写入封装头
        ret = avformat_write_header(ic, NULL);
        if(ret != 0)
        {
            char buf[1024] = { 0 };
            av_strerror(ret, buf, sizeof(buf) - 1);
            this->setLastError(buf);
            return false;
        }
        return true;
    }

    bool sendFrame(AVPacket* pack, int index)
    {
        if(pack->size <= 0 || !pack->data)
            return false;
        const AVRational* stime = NULL;
        const AVRational* dtime = NULL;
        pack->stream_index = index;
        //判断是音频还是视频
        if(vs && vc && pack->stream_index == vs->index)
        {
            stime = &(vc->time_base);
            dtime = &(vs->time_base);
        }
        else if(as && ac && pack->stream_index == as->index)
        {
            stime = &(ac->time_base);
            dtime = &(as->time_base);
        }
        else
        {
            return false;
        }

        // 推流 a*b / c
        // pack->pts * vc->time_base 为实际秒数
        pack->pts = av_rescale_q(pack->pts, *stime, *dtime);
        pack->dts = av_rescale_q(pack->dts, *stime, *dtime);
        pack->duration = av_rescale_q(pack->duration, *stime, *dtime);
        int ret = av_interleaved_write_frame(ic, pack);
        av_packet_unref(pack);
        if(ret == 0)
            return true;

        return false;
    }

    void setLastError(const std::string& buf)
    {
        err_msg = buf;
    }
    const std::string& getLastError()
    {
        return err_msg;
    }
private:
    //rtmp flv 封装器
    AVFormatContext* ic = NULL;

    // 视频编码器流
    const AVCodecContext* vc = NULL;
    AVStream* vs = NULL;

    // 音频编码器流
    const AVCodecContext* ac = NULL;
    AVStream* as = NULL;

    std::string url;
    std::string err_msg;
};

//工厂生产方法
XRtmp* XRtmp::getInstance(unsigned char index)
{
    static CXRtmp cxr[255];

    static bool is_first = true;
    if(is_first)
    {
        //注册所有网络协议
        avformat_network_init();
        is_first = false;
    }
    return &cxr[index];
}


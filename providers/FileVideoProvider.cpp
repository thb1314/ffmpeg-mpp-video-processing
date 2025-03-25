#include <chrono>
#include <iostream>
#include "FileVideoProvider.h"
#include "Utils.h"

const std::map<std::string, std::string> FileVideoProvider::decoder_map = {
    {"h264", "h264_rkmpp"},
    {"libx264", "h264_rkmpp"},
    {"h265", "hevc_rkmpp"},
    {"hevc", "hevc_rkmpp"},
    {"libx265", "hevc_rkmpp"},
};

FileVideoProvider::FileVideoProvider(const char *url) : url(url), VideoProvider(VideoType::File)
{
    max_queue_len = 100;
}

// 初始化操作
bool FileVideoProvider::init()
{
    // 初始化网络
    avformat_network_init();

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    // 网络延时时间
    av_dict_set(&opts, "max_delay", "500", 0);
    // 打开视频流
    if (avformat_open_input(&formatCtx, url.c_str(), NULL, &opts) != 0)
    {
        std::cerr << "Failed to open input stream." << std::endl;
        av_dict_free(&opts);
        return false;
    }
    av_dict_free(&opts);

    // 获取流信息
    if (avformat_find_stream_info(formatCtx, NULL) < 0)
    {
        std::cerr << "Failed to find stream info." << std::endl;
        return false;
    }
    // 打印视频流详细信息
    av_dump_format(formatCtx, 0, url.c_str(), 0);
    // 查找视频流
    for (int i = 0; i < formatCtx->nb_streams; i++)
    {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1)
    {
        std::cerr << "Failed to find video stream." << std::endl;
        return false;
    }

    // 查找解码器
    codec = avcodec_find_decoder(formatCtx->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec)
    {
        std::cerr << "Failed to find codec." << std::endl;
        return false;
    }

    // 尝试寻找硬解码
    bool use_hard_decoder = false;
    bool can_not_hard_decode = (url.substr(0, 4) == "rtsp" && std::string(codec->name).substr(0, 4) == "hevc");
    if (!can_not_hard_decode)
    {
        auto decoder_name = std::string(codec->name);
        auto it = decoder_map.find(decoder_name);
        if (it != decoder_map.end())
        {
            auto &try_decoder_name = it->second;
            const AVCodec *tmp_codec = avcodec_find_decoder_by_name(try_decoder_name.c_str());
            if (tmp_codec)
            {
                codec = tmp_codec;
                use_hard_decoder = true;
            }
        }
    }

    std::cout << "video decoder name: " << codec->name << std::endl;

    // 根据解码器 创建解码器上下文
    codecCtx = avcodec_alloc_context3(codec);

    if (!codecCtx)
    {
        std::cerr << "Failed to allocate codec context." << std::endl;
        return false;
    }
    codecCtx->thread_count = use_hard_decoder ? 1 : Utils::core_count();
    // 配置解码器上下文
    if (avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIndex]->codecpar) < 0)
    {
        std::cerr << "Failed to copy codec parameters." << std::endl;
        return false;
    }

    // 打开解码器
    if (avcodec_open2(codecCtx, codec, NULL) < 0)
    {
        std::cerr << "Failed to open codec." << std::endl;
        return false;
    }

    // 设置视频流参数
    width = codecCtx->width;
    height = codecCtx->height;
    fps = codecCtx->framerate.num / codecCtx->framerate.den; // fps = num/den
    std::cout << "decoding video width:" << width << std::endl;
    std::cout << "decoding video height:" << height << std::endl;
    std::cout << "decoding video fps:" << fps << std::endl;
    return true;
}

void FileVideoProvider::stop()
{
    VideoProvider::stop();
    std::lock_guard<std::mutex> lock(mutex);
    if (codecCtx)
    {
        avcodec_free_context(&codecCtx);
    }
    if (formatCtx)
    {
        avformat_close_input(&formatCtx);
    }
    if (swsCtx)
    {
        sws_freeContext(swsCtx);
    }
    swsCtx = nullptr;
    codecCtx = nullptr;
    formatCtx = nullptr;
    codec = nullptr;
}

void FileVideoProvider::run()
{
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    uint8_t *buffer = nullptr;

    // 设置输出RGB格式的帧
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);

    // 使用 SwsContext 将 YUV 转为 RGB
    swsCtx = sws_getContext(width, height, codecCtx->pix_fmt,
                            width, height, AV_PIX_FMT_RGB24,
                            SWS_BICUBIC, nullptr, nullptr, nullptr);
    int try_time = 0;
    int ret = -1;
    std::cout << "a1" << std::endl;
    int64_t frame_count = 0;
    while (!is_exit)
    {
        av_packet_unref(pkt);
        // 发送数据包给解码器
        ret = av_read_frame(formatCtx, pkt);
        if (AVERROR_EOF == ret)
        {
            ret = avcodec_send_packet(codecCtx, NULL);
            if (0 != ret)
                break;
        }
        else if (0 != ret)
        {
            try_time++;
            if (try_time > 100 || is_exit)
                break;
            continue;
        }
        try_time = 0;
        if (pkt->stream_index != videoStreamIndex)
            continue;

        if (avcodec_send_packet(codecCtx, pkt) < 0)
        {
            std::cerr << "Error sending packet to decoder" << std::endl;
            continue;
        }
        // 获取解码后的帧
        while (!is_exit)
        {
            int ret = avcodec_receive_frame(codecCtx, frame);
            if (0 != ret)
                break;

            frame_count += 1;
            if (frame_count % frame_interval != 0)
            {
                av_frame_unref(frame);
                break;
            }

            // 转换为 RGB 格式
            int len = sws_scale(swsCtx, frame->data, frame->linesize, 0, height, rgbFrame->data, rgbFrame->linesize);
            if (len <= 0)
            {
                av_frame_unref(frame);
                continue;
            }

            // 计算时间戳，转换为微秒
            int64_t pts = frame->best_effort_timestamp;
            int64_t timestamp_us = av_q2d(formatCtx->streams[videoStreamIndex]->time_base) * 1000000.0 * pts;
            if (timestamp_us == 0)
            {
                timestamp_us = frame_count * 1000000.0 / fps;
            }

            while (!is_exit && data_queue.size() > max_queue_len / 3 * 2)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            push(FramePtrWrapper(rgbFrame->data[0], height * width * 3, timestamp_us));

            // 清理帧数据
            av_frame_unref(frame);
            break;
        }
    }

    // 清理资源
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_freep(&buffer);
    is_exit = true;
}

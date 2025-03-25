#include "VideoProvider.h"
#include <exception>

VideoProvider::VideoProvider(VideoProvider::VideoType type) : type(type)
{
}

VideoProvider::~VideoProvider()
{
    stop();
}

int VideoProvider::getFps() const
{
    return fps / frame_interval;
}

int VideoProvider::getHeight() const
{
    return height;
}

int VideoProvider::getWidth() const
{
    return width;
}

int VideoProvider::getFrameInterval() const
{
    return frame_interval;
}

bool VideoProvider::setFrameInterval(const int interval)
{
    if (interval <= 0)
        return false;
    frame_interval = interval;
    return true;
}

# Project Introduction
[View Chinese README](README.md)
This project is a video processing project based on FFmpeg and Rockchip MPP, which mainly implements the functions of hardware decoding, hardware encoding, streaming of video files, and writing to local MP4 files.

## Features
1. **Video Decoding**: Supports the decoding of multiple video formats, including H.264 and H.265.
2. **Video Encoding**: Supports encoding the decoded video frames into H.264 format.
3. **Video Streaming**: Supports pushing the encoded video stream to an RTMP server or writing to a local MP4 file.

## Usage
### Compile the Project
Use CMake to compile the project:
```sh
mkdir build
cd build
cmake ..
make
```
### Run the Project
Run the executable file in the `build` directory:
```sh
./ffmpeg_demo
```

## Notes
- Please ensure that FFmpeg and related dependency libraries are installed on the system.
- Please modify the input video file path and RTMP server address in `main.cpp` according to the actual situation (you can directly modify it to a local file address).

## Project Structure
- `core`: Core function module, including data encapsulation and utility functions.
- `encoders`: Encoder module, including video encoder and RTMP streamer (local file writer).
- `providers`: Video provider module, including file video provider.
- `main.cpp`: Project entry file.


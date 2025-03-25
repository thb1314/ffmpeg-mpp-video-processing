#ifndef FRAMEPTRWRAPPER_H
#define FRAMEPTRWRAPPER_H

#include <cstdint>

/**
 * @class FramePtrWrapper
 * @brief 用于封装数据指针及其相关信息的类，提供了数据管理和操作的功能。
 * 
 * 该类管理一个指向数据的指针，同时记录数据的字节大小和时间戳。
 * 支持深拷贝、移动语义等操作，确保数据的安全管理和高效使用。
 */
class FramePtrWrapper
{
private:
    void* data_ptr = nullptr;  // 指向数据的指针，初始化为空指针
    int byte_size = 0;         // 数据的字节大小，初始化为 0
    int64_t timestamp = -1;    // 数据的时间戳，初始化为 -1，表示无效时间戳

public:
    /**
     * @brief 默认构造函数
     * 以默认值初始化对象的成员变量。
     */
    FramePtrWrapper() = default;

    /**
     * @brief 构造函数
     * 使用指定的数据指针、字节大小和时间戳初始化对象。
     * 
     * @param data_ptr 指向数据的指针
     * @param byte_size 数据的字节大小
     * @param timestamp 数据的时间戳，默认为 -1
     */
    FramePtrWrapper(void* data_ptr, int byte_size, int64_t timestamp = -1);

    /**
     * @brief 构造函数
     * 使用指定的字节大小初始化对象，数据指针初始化为空。
     * 
     * @param byte_size 数据的字节大小
     */
    FramePtrWrapper(int byte_size);

    /**
     * @brief 深拷贝构造函数
     * 创建一个新对象，复制另一个对象的数据。
     * 
     * @param other 要复制的对象
     */
    // 深拷贝 拷贝构造
    FramePtrWrapper(const FramePtrWrapper& other);

    /**
     * @brief 赋值运算符重载
     * 将另一个对象的数据复制到当前对象。
     * 
     * @param other 要复制的对象
     * @return FramePtrWrapper& 返回当前对象的引用
     */
    // 赋值构造
    FramePtrWrapper& operator=(const FramePtrWrapper& other);

    /**
     * @brief 移动构造函数
     * 使用移动语义创建一个新对象，接管另一个对象的资源。
     * 
     * @param other 要移动的对象
     */
    // 移动构造
    FramePtrWrapper(FramePtrWrapper&& other);

    /**
     * @brief 移动赋值运算符重载
     * 使用移动语义将另一个对象的资源转移到当前对象。
     * 
     * @param other 要移动的对象
     * @return FramePtrWrapper& 返回当前对象的引用
     */
    // 赋值移动构造
    FramePtrWrapper& operator=(FramePtrWrapper&& other);

    /**
     * @brief 交换两个对象的资源
     * 交换当前对象和另一个对象的数据指针、字节大小和时间戳。
     * 
     * @param other 要交换的对象
     */
    // bind
    void swap(FramePtrWrapper& other);

    /**
     * @brief 析构函数
     * 释放对象管理的资源。
     */
    ~FramePtrWrapper();

    /**
     * @brief 设置数据指针和字节大小
     * 
     * @param data_ptr 指向数据的指针
     * @param byte_size 数据的字节大小
     */
    void setDataPtr(void* data_ptr, int byte_size);

    /**
     * @brief 直接设置数据指针和字节大小
     * 
     * @param data_ptr 指向数据的指针
     * @param byte_size 数据的字节大小
     */
    void bindDataPtr(void* data_ptr, int byte_size);

    /**
     * @brief 获取数据指针
     * 
     * @return void* 指向数据的指针
     */
    void* getDataPtr() const;

    /**
     * @brief 调整数据的字节大小
     * 
     * @param byte_size 新的数据字节大小
     */
    void resize(int byte_size);

    /**
     * @brief 获取数据的时间戳
     * 
     * @return int64_t 数据的时间戳
     */
    int64_t getTimestamp() const;

    /**
     * @brief 设置数据的时间戳
     * 
     * @param value 新的时间戳值
     */
    void setTimestamp(int64_t value);

    /**
     * @brief 获取数据的字节大小
     * 
     * @return int 数据的字节大小
     */
    int getByteSize() const;
};

#endif // FRAMEPTRWRAPPER_H

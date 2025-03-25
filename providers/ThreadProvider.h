#ifndef THREADPROVIDER_H
#define THREADPROVIDER_H

#include <thread>
#include <list>
#include <mutex>
#include "FramePtrWrapper.h"

/**
 * @brief 线程提供者基类
 *
 * 该类提供了一个基于线程的数据提供机制，用于管理数据队列的操作，
 * 如数据的入队、出队，以及线程的启动和停止等功能。
 * 派生类需要实现 run 方法来定义线程的具体执行逻辑。
 */
class ThreadProvider
{
public:
    /**
     * @brief 默认构造函数
     * 显式要求编译器生成 ThreadProvider 类的默认构造函数，
     * 以默认方式初始化类的成员变量。
     */
    ThreadProvider() = default;

    /**
     * @brief 获取队列的最大长度
     * 该函数用于获取数据队列允许存储的最大元素数量。
     * 由于使用了 const 修饰，该函数不会修改类的成员变量。
     *
     * @return int 队列的最大长度
     */
    int getMaxQueueLength() const;

    /**
     * @brief 设置数据队列的最大长度
     *
     * 该方法用于修改数据队列允许存储的最大元素数量。
     * 调用此方法后，`data_queue` 队列在达到新的最大长度后可能会有相应的处理逻辑（如阻塞入队操作）。
     *
     * @param value 要设置的队列最大长度值
     */
    void setMaxQueueLength(int value);

    /**
     * @brief 线程提供者类的析构函数
     *
     * 这是一个虚析构函数，确保在删除指向派生类对象的基类指针时，能够正确调用派生类的析构函数，
     * 避免内存泄漏。在析构过程中，可能会执行停止线程、清理队列等资源释放操作。
     */
    virtual ~ThreadProvider();

    /**
     * @brief 向数据队列中插入一个左值引用的帧数据
     *
     * 该方法将一个 `FramePtrWrapper` 类型的左值引用插入到数据队列中。
     * 派生类可以重写此方法以实现自定义的插入逻辑。
     *
     * @param d 要插入的 `FramePtrWrapper` 类型的左值引用
     */
    virtual void push(const FramePtrWrapper &d);

    /**
     * @brief 向数据队列中插入一个右值引用的帧数据
     *
     * 该方法使用移动语义将一个 `FramePtrWrapper` 类型的右值引用插入到数据队列中，
     * 避免不必要的拷贝操作，提高性能。派生类可以重写此方法以实现自定义的插入逻辑。
     *
     * @param d 要插入的 `FramePtrWrapper` 类型的右值引用
     */
    virtual void push(FramePtrWrapper &&d);

    /**
     * @brief 从数据队列中取出最早放入的数据
     *
     * 该方法会移除并返回数据队列中最早放入的 `FramePtrWrapper` 对象。
     * 返回的数据无需手动清理，由 `FramePtrWrapper` 自身管理资源。
     * 派生类可以重写此方法以实现自定义的取出逻辑。
     *
     * @return FramePtrWrapper 数据队列中最早放入的 `FramePtrWrapper` 对象
     */
    virtual FramePtrWrapper pop();

    /**
     * @brief 查看数据队列中最早放入的数据
     *
     * 该方法会返回数据队列中最早放入的 `FramePtrWrapper` 对象，但不会将其从队列中移除。
     * 返回的数据无需手动清理。派生类可以重写此方法以实现自定义的查看逻辑。
     *
     * @return FramePtrWrapper 数据队列中最早放入的 `FramePtrWrapper` 对象
     */
    virtual FramePtrWrapper top();

    /**
     * @brief 启动线程
     *
     * 该方法用于启动线程，开始执行 `run` 方法中定义的逻辑。
     * 派生类可以重写此方法以实现自定义的线程启动逻辑。
     */
    virtual void start();

    /**
     * @brief 停止线程并等待线程退出
     *
     * 该方法会请求线程退出，并阻塞当前线程直到目标线程完全退出。
     * 派生类可以重写此方法以实现自定义的线程停止逻辑。
     */
    virtual void stop();

    /**
     * @brief 线程启动后执行的核心函数
     *
     * 这是一个纯虚函数，派生类必须实现该方法，以定义线程启动后要执行的具体逻辑。
     */
    virtual void run() = 0;

    /**
     * @brief 判断线程是否正在运行
     *
     * 该方法通过检查 `is_exit` 标志来判断线程是否正在运行。
     *
     * @return const bool 如果线程正在运行返回 `true`，否则返回 `false`
     */
    inline const bool isRunning() const { return is_exit == false; }

protected:
    /**
     * @brief 线程对象
     * 用于执行 `run` 方法中定义的线程逻辑，派生类需要实现 `run` 方法以确定线程的具体行为。
     */
    std::thread m_thread;
    /**
     * @brief 存放交互数据的队列，采用先进先出（FIFO）策略
     * 该队列用于存储 `FramePtrWrapper` 类型的数据，新数据会被添加到队列尾部，
     * 取出数据时从队列头部获取，实现数据的有序处理。
     */
    std::list<FramePtrWrapper> data_queue;
    /**
     * @brief 互斥锁，用于保护对数据队列的访问
     * 当多个线程需要同时访问 `data_queue` 时，使用该互斥锁可以确保同一时间只有一个线程能够操作队列，
     * 避免数据竞争和不一致的问题。
     */
    std::mutex mutex;
    /**
     * @brief 当前数据队列的大小
     * 记录 `data_queue` 中当前存储的 `FramePtrWrapper` 对象的数量，方便监控队列状态。
     */
    int curQueueSize = 0;
    /**
     * @brief 数据队列的最大长度
     * 限制 `data_queue` 中能够存储的 `FramePtrWrapper` 对象的最大数量，默认值为 100。
     */
    int max_queue_len = 100;
    /**
     * @brief 线程退出标志
     * 用于控制线程的运行状态，当 `is_exit` 为 `true` 时，表示线程需要退出；
     * 为 `false` 时，表示线程正在运行。
     */
    bool is_exit = true;
};

#endif // THREADPROVIDER_H

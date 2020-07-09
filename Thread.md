

`MutexLock`是对加锁/解锁的封装

`MutexLockGuard`能够在作用域内自动解锁

```c++
// 实例化
muduo::MutexLock mutex;

// 加锁/解锁
mutex.lock();
mutex.unlock();

// 判断使用互斥锁的当前线程是否是加锁时的线程
mutex.isLockedByThisThread();

// 获得mutex值，仅供Condition调用，用于初始化cond
mutex.getPthreadMutex();


// 实例化并加锁
muduo::MutexLockGuard mutexGuard(mutex);
```



# 条件变量 Condition

```c++
// 实例化
muduo::Condition cond(mutex);

// 等待通知
cond.wait();

// 通知任意一个线程
cond.notify();

// 通知所有线程
cond.notifyAll();
```



# 线程 Thread

```c++
// 创建线程
muduo::Thread thread(函数, 函数参数);

// 启动线程
thread.start();                                                                                                                                                                                 
```

​                                                                  
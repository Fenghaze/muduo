# 线程标识符

- Linux中每个进程有一个`pid_t`类型的进程号`pid`，可由`getpid()`获得

- Linux中的线程称轻量级线程（LWP），POSIX线程有一个`pthread_t`类型的`id`，可由`pthread_self()`获得
  - 不同进程中的线程可能有相同的`id`
- 两个进程的线程使用信号进行通信时，不能使用所在进程的`pid`或线程`pthread_id`来发送信号，只能使用真实的`pid`即`tid`进行通信
  - 当一个进程只有一个线程时，`pid=tid`
- `tid`可以使用`gettid()`获得，glibc并没有实现该函数，只能**通过系统调用`syscall`来获取**
  - `return syscall(SYS_gettid)`



# 多线程和fork

当一个进程中有多个线程时，可能会存在两种fork情况：

- fork在main线程中调用：fork出的子进程会复制整个父进程的内存
- fork在其他线程中调用：fork出的子进程只继承了调用fork的线程函数，即只有一个线程，这个线程也是当前子进程中的main线程

:boom:在实际项目中，多线程调用fork容易造成死锁；==尽量不要同时使用多进程多线程，要么用多进程要么用多线程==

> 死锁例子

```c++
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *threadfunc(void *arg)
{
    printf("this is a threadfunc %d\n", getpid());
    pthread_mutex_lock(&mutex);
    sleep(2);
    printf("threadfunc working... %d\n", getpid());
    pthread_mutex_unlock(&mutex);
    printf("this is a threadfunc exit %d\n", getpid());
    return NULL;
}
int main()
{
    printf("this is a mainthread\n");
    pthread_t pthread_id;
    pthread_create(&pthread_id, NULL, threadfunc, NULL);
    sleep(1);
    if (fork() == 0)	//fork一个子进程，子进程调用threadfunc，子进程复制当前线程的执行状态，此时mutex是处于锁状态
    {
        threadfunc(NULL);
    }
    pthread_join(pthread_id, NULL);
    pause();
    printf("this is a mainthread exit\n");
    return 0;
}
```

> :bell:使用`ptrhead_atfork`防止死锁
>
> 函数意义：调用fork时，内部创建子进程前在父进程会调用prepare，内部创建子进程成功后，父进程会调用parent，子进程会调用child

```c++
void prepare(void)
{
    pthread_mutex_unlock(&mutex);
}

void parent(void)
{
    pthread_mutex_lock(&mutex);
}
int main()
{
    pthread_atfork(prepare, parent, NULL);
   	...
    return 0;
}
```



# Thread.h

- 作用：
  - 封装线程类
  - 使用`std::function`来实现**基于对象编程的线程函数调用**
- 一些成员变量

|              变量              |                            描述                             |
| :----------------------------: | :---------------------------------------------------------: |
|      pthread_t pthreadId_      |                         POSIX线程id                         |
|          pid_t   tid_          |                         LWP线程tid                          |
|      **ThreadFunc func_**      | **typedef std::function<void ()> ThreadFunc；线程执行函数** |
|     CountDownLatch latch_      |                主线程和子线程的任务倒计时？                 |
| static AtomicInt32 numCreated_ |                静态原子类型，已启动的线程数                 |

- 一些成员函数

|     函数     |                             描述                             |
| :----------: | :----------------------------------------------------------: |
| void start() | 调用pthread_create(&pthreadId_, NULL, **&detail::startThread**, data) |
|  int join()  |              调用pthread_join(pthreadId_, NULL)              |



# Thread.cc

源文件中除了实现`Thread.h`的成员函数，还定义了一些线程相关的东西

- 一些函数

|                   函数                   |                      描述                       |
| :--------------------------------------: | :---------------------------------------------: |
|          detail::pid_t gettid()          |     通过系统调用`syscall`来获取线程真实tid      |
|             void afterFork()             |             多线程环境下fork子进程              |
| **void* detail::startThread(void* obj)** | **线程入口函数，调用ThreadData::runInThread()** |

- 定义的类和结构体

|          类\结构体          |                             描述                             |
| :-------------------------: | :----------------------------------------------------------: |
| class ThreadNameInitializer | 初始化当前线程名和线程tid，执行pthread_atfork(NULL, NULL, &afterFork) |
|    **struct ThreadData**    |          **设置Thread::ThreadFunc；并执行线程函数**          |

> struct ThreadData
>

```c++
struct ThreadData
{
    typedef muduo::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;
    ThreadData(ThreadFunc func, const string& name, pid_t* tid, CountDownLatch* latch): func_(std::move(func)), name_(name), tid_(tid), latch_(latch)
    { }
	//核心是执行线程函数
    void runInThread()
    {
        ...	
        func_();
        ...
    }
};
```

> Thread_test.cc

```c++
void threadFunc()
{
    printf("tid=%d\n", muduo::CurrentThread::tid());
}

void threadFunc2(int x)
{
    printf("tid=%d, x=%d\n", muduo::CurrentThread::tid(), x);
}

int main()
{
    muduo::Thread t1(threadFunc);	//传入void()类型函数，不需要使用bind适配接口
    t1.start();
    printf("t1.tid=%d\n", t1.tid());
    t1.join();
    
    muduo::Thread t2(std::bind(threadFunc2, 42), "thread for free function with argument");		//传入void(int)类型函数，需要使用bind适配接口
    t2.start();
    printf("t2.tid=%d\n", t2.tid());
    t2.join();
}
```



# CurrentThread.h

- 作用：
  - 当前线程的**环境**，包含了当前线程的上下文环境
- 一些变量

|               变量                |                           描述                            |
| :-------------------------------: | :-------------------------------------------------------: |
|     __thread int t_cachedTid      | 线程真实tid的缓存（如果每次调用gettid来获得，会降低效率） |
|   __thread char t_tidString[32]   |                     tid格式化为字符串                     |
|  __thread int t_tidStringLength   |                       tid字符串长度                       |
| __thread const char* t_threadName |                         线程名称                          |

> :exclamation:__thread关键字
>
> __thread，是gcc内置的**线程局部存储设施**，只能用来修饰POD类型（与C兼容的原始数据）
>
> POD类型：结构体和整型等C语言中的类型是POD类型
>
> 非POD类型：调用了构造函数的对象；非编译期常量
>
> 作用：一个进程含有多个线程，==使用__thread修饰的变量能够仅保存当前线程的信息（线程私有）==，如果不用\_\_thread，则这些变量就是全局的，即线程共享

- 一些函数

|               函数               |           描述           |
| :------------------------------: | :----------------------: |
|            int tid()             |       返回线程tid        |
|         void cacheTid()          |         缓存tid          |
|       bool isMainThread()        |    判断是否是main线程    |
| string stackTrace(bool demangle) | 获得调用栈中的所有函数名 |

```c++
string stackTrace(bool demangle)
{
    string stack;
    const int max_frames = 200;		//函数栈帧个数
    void* frame[max_frames];		//存放栈帧的数据结构
    int nptrs = ::backtrace(frame, max_frames);	//获取实际栈帧个数
    char** strings = ::backtrace_symbols(frame, nptrs);	//根据栈帧地址，转换成相应的函数符号
    //遍历strings，把栈帧对应的函数名添加到stack中
    if (strings)
    {
        size_t len = 256;
        char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
        for (int i = 1; i < nptrs; ++i)  // skipping the 0-th, which is this function
        {
            if (demangle)	//是否还原真实的函数名，默认为false
            {
                //核心是调用cabi::__cxa_demangle()进行还原
            }
            // Fallback to mangled names
            stack.append(strings[i]);
            stack.push_back('\n');
        }
        free(demangled);
        free(strings);
    }
    return stack;
}
```



# TreadPool.h

- 作用：
  - 与【BoundedBlockQueue.h】一样，实质是一个**有界的生产者-消费者模型**
  - 创建一个任务队列，生产者使用`run()`来生产线程任务
  - 创建一个线程池，里面包含多个消费者线程使用`take()`来执行线程任务

- 成员变量：

|                       变量                       |                          描述                           |
| :----------------------------------------------: | :-----------------------------------------------------: |
|             mutable MutexLock mutex_             |               保护线程池任务队列的互斥锁                |
|               Condition notEmpty_                |                        条件变量1                        |
|                Condition notFull_                |                        条件变量2                        |
|           **Task threadInitCallback_**           | **typedef std::function<void ()> Task**；线程执行的任务 |
| **vector\<unique_ptr\<muduo::Thread>> threads_** |                         线程池                          |
|             **deque\<Task> queue_**              |                        任务队列                         |
|               size_t maxQueueSize_               |                        最大长度                         |
|                  bool running_                   |                      线程运行flag                       |

- 一些成员函数：

|                    函数                    |                    描述                    |
| :----------------------------------------: | :----------------------------------------: |
| void setThreadInitCallback(const Task& cb) |      设置线程池中的每个线程执行的任务      |
|       **void start(int numThreads)**       |         启动线程池：创建消费者线程         |
|                void stop()                 |          关闭线程池：回收所有线程          |
|            **void run(Task f)**            |  生产者：向**非满**的任务队列添加线程任务  |
|            bool isFull() const             |               判断队列是否满               |
|           **void runInThread()**           |       使用take()获得一个任务，并执行       |
|              **Task take()**               | 消费者：从**非空**的任务队列中取出线程任务 |



# 线程特定数据

**线程共享数据：**一个进程有多个线程，线程间除了内核栈和用户栈空间的数据共享以外，其他都是共享的

但有时，应用程序中有必要提供**线程私有的全局变量，仅作用在当前线程，却也可以跨多个函数访问**

POSIX线程库使用**TSD（Thread-specific Data）**这个数据结构来表示线程特定数据

线程特定数据也称为**线程本地存储TLS（Thread-local storage）**

对于**POD类型**（C数据类型）的线程本地存储，可以用**\_\_thread关键字**



**POSIX线程库使用下面一系列函数来操作TSD**

```c++
//在一个线程中创建一个key，表示该线程可拥有特定数据
int pthread_key_create(pthread_key_t *key, void (*destr_function) (void *));

//设置特定数据
int pthread_setspecific(pthread_key_t key, const void *pointer);

//获取特定数据
void * pthread_getspecific(pthread_key_t key);

//删除key
int pthread_key_delete(pthread_key_t key);
```



# ThreadLocal.h

- 作用：

  - 模板类

  - 实质上封装POSIX线程库提供的`pthread_key_create()`系列函数，用于**创建当前线程的特定数据**

- 成员变量

|        变量         | 描述 |
| :-----------------: | :--: |
| pthread_key_t pkey_ | key  |

- 一些成员函数

|              函数               |             描述              |
| :-----------------------------: | :---------------------------: |
|           T& value()            | 获得线程特定数据，转换为T类型 |
| static void destructor(void *x) |         销毁特定数据          |



# ThreadLocalSingleton.h

- 作用：

  - 模板类
  - 线程特定数据类的单例模式
  - 使用了两种线程特定数据存储机制
    - （1）POD类型的线程本地存储，使用`__thread`关键字创建线程特定数据
    - （2）POSIX线程库TSD数据结构，使用一个`Deleter `类来封装

- 成员变量

|              变量               |         描述          |
| :-----------------------------: | :-------------------: |
| static \_\_thread T* t\_value\_ | POD类型的线程特定数据 |
|    static Deleter deleter\_     |   **包含**的删除类    |

- 一些成员函数

|               函数                |           描述           |
| :-------------------------------: | :----------------------: |
|       static T& instance()        | 返回特定数据（单例模式） |
|        static T* pointer()        |    返回特定数据的指针    |
| static void destructor(void* obj) |       删除特定数据       |

- Deleter类：封装的POSIX线程库操作TSD的函数

```c++
class Deleter
{
public:
    Deleter()
    {
        pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
    }

    ~Deleter()
    {
        pthread_key_delete(pkey_);
    }

    void set(T* newObj)
    {
        assert(pthread_getspecific(pkey_) == NULL);
        pthread_setspecific(pkey_, newObj);
    }

    pthread_key_t pkey_;
};
```


# Singleton.h

- 作用：
  - 模板类+线程安全单例模式
  - 核心是使用`int pthread_once(pthread_once_t *once_control, void (*init_routine) (void))`
    - 功能：本函数使用初值为PTHREAD_ONCE_INIT的once_control变量**保证init_routine()函数在本进程执行序列中仅执行一次**

> :exclamation:**pthread_once的应用场景：**
>
> 在多线程环境中，有些事仅需要执行一次。通常当初始化应用程序时，可以比较容易地将其放在main函数中。但当你写一个库时，就不能在main里面初始化了，你可以用静态初始化，但使用一次初始化（pthread_once）会比较容易些
>
> **保证线程安全：在多线程编程环境下，可能会有多个线程调用pthread_once()，但是init_routine()函数仅执行一次**
>
> Linux Threads使用互斥锁和条件变量保证由pthread_once()指定的函数执行且仅执行一次，而once_control表示是否执行过
>
> 在LinuxThreads中，实际"一次性函数"的执行状态有三种：NEVER（0）、IN_PROGRESS（1）、DONE （2），如果once_control初值设为1，则由于所有pthread_once()都必须等待其中一个激发"已执行一次"信号，因此所有pthread_once ()都会陷入永久的等待中；如果设为2，则表示该函数已执行过一次，从而所有pthread_once()都会立即返回0

- 成员变量

|             变量             |          描述          |
| :--------------------------: | :--------------------: |
| static pthread_once_t ponce_ | 保证任务函数只执行一次 |
|       static T* value_       |       T类型指针        |

- 成员函数

|         函数          |                       描述                        |
| :-------------------: | :-----------------------------------------------: |
| static T& instance()  | 调用pthread_once()执行一次init()；返回对象T的实例 |
|  static void init()   |                  初始化一个T对象                  |
| static void destroy() |                     销毁T对象                     |


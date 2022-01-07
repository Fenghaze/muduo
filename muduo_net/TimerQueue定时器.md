定时器实现的逻辑：

- 定时器容器可以是一个升序链表、时间轮、时间堆
- 容器中的每个节点保存了截止时间、定时回调等
- Linux提供了一系列系统调用的定时函数，用于让程序等待一段时间或安排计划任务：
  - sleep、usleep、alarm
  - nanosleep、clock_nanosleep
  - getitimer/setitimer
  - timer_create/timer_settime/timer_gettime/timer_delete
  - **timerfd_create/timerfd_gettime/timerfd_settime**



# 定时器函数选择

timerfd_ *入选的原因：

- sleep / alarm / usleep 在实现时有可能用了信号 SIGALRM，**在多线程程序中处理信号是个相当麻烦的事情**，**应当尽量避免**
- nanosleep 和 clock_nanosleep 是线程安全的，但是在非阻塞网络编程中，绝对不能用让线程挂起的方式来等待一段时间，程序会失去响应。正确的做法是注册一个时间回调函数。 
- getitimer 和 timer_create 也是用信号来 deliver 超时，在多线程程序中也会有麻烦。
- timer_create 可以指定信号的接收方是进程还是线程，算是一个进步，不过在信号处理函数(signal handler)能做的事情实在很受限。 
- **timerfd_create 把时间变成了一个文件描述符，该“文件”在定时器超时的那一刻变得可读，这样就能很方便地融入到 select/poll 框架中，用统一的方式来处理 IO 事件和超时事件，这也正是 Reactor 模式的长处。**

```c++
#include <sys/timerfd.h>
//创建一个定时器，返回其文件描述符
int timerfd_create(int clockid, int flags);
//为定时器设置一个超时时间
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

int timerfd_gettime(int fd, struct itimerspec *curr_value)
```



# Timer

- 作用：
  - TimerQueue定时器容器的节点

- 一些成员变量：

|            变量             |                           描述                            |
| :-------------------------: | :-------------------------------------------------------: |
| **TimerCallback callback_** | typedef std::function<void()> TimerCallback；定时回调函数 |
|  **Timestamp expiration_**  |                       定时截止时间                        |
|   const double interval_    |                         时间间隔                          |

- 一些成员函数：

|            函数             |                  描述                   |
| :-------------------------: | :-------------------------------------: |
|    **void run() const**     |            执行定时回调函数             |
| void restart(Timestamp now) | 重启定时器，在现有的时间上增加interval_ |



# TimerId

- 作用：
  - 返回定时器节点在TimerQueue中的位置

```c++
class Timer;
class TimerId : public muduo::copyable
{
 public:
  TimerId(): timer_(NULL),sequence_(0){}
  TimerId(Timer* timer, int64_t seq): timer_(timer),sequence_(seq){}
  friend class TimerQueue;
 private:
  Timer* timer_;
  int64_t sequence_;
};
#endif  // MUDUO_NET_TIMERID_H
```



# TimerQueue

- 作用：
  - 定时器容器

- 一些成员变量：

|                      变量                      |                            描述                             |
| :--------------------------------------------: | :---------------------------------------------------------: |
|   typedef std::pair<Timestamp, Timer*> Entry   |                     定时容器节点，pair                      |
| typedef std::pair<Timer*, int64_t> ActiveTimer |                        激活定时节点                         |
|              **EventLoop* loop_**              |                       所在的事件循环                        |
|             **const int timerfd_**             |                  设置了定时器的文件描述符                   |
|            Channel timerfdChannel_             |                        定时事件通道                         |
|             **TimerList timers_**              |        typedef std::set\<Entry> TimerList；定时容器         |
|        **ActiveTimerSet activeTimers_**        | typedef std::set\<ActiveTimer> ActiveTimerSet；激活定时容器 |
|           bool callingExpiredTimers_           |                      是否启动定时事件                       |
|        ActiveTimerSet cancelingTimers_         |                        取消定时事件                         |

- 一些成员函数：

|                             函数                             |                             描述                             |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
|              void addTimerInLoop(Timer* timer)               |           向loop_中设置一个定时器，内部调用insert            |
| **TimerId addTimer(TimerCallback cb, Timestamp when, double interval)** | 创建一个Timer对象，调用loop_->runInLoop()设置回调函数为addTimerInLoop()，并返回Timer对象所在容器的位置TimerId |
|               **void cancel(TimerId timerId)**               |      调用loop_->runInLoop()设置回调函数为cancelInLoop()      |
|              void cancelInLoop(TimerId timerId)              |                从定时器容器中删除一个定时节点                |
|                    **void handleRead()**                     |          执行定时器每个节点的回调，并调用reset重置           |
|        std::vector\<Entry> getExpired(Timestamp now)         |               获取定时容器中所有节点的截止日期               |
| **void reset(const std::vector\<Entry>& expired, Timestamp now)** |                   重置定时容器中的所有节点                   |
|                  bool insert(Timer* timer)                   |                往定时器容器中添加一个定时节点                |

- 其他函数：

|                         函数                         |                 描述                 |
| :--------------------------------------------------: | :----------------------------------: |
|                 int createTimerfd()                  | timerfd_create()创建定时器文件描述符 |
|     void readTimerfd(int timerfd, Timestamp now)     |         读取定时器文件描述符         |
| void resetTimerfd(int timerfd, Timestamp expiration) |     timerfd_settime()重置定时器      |
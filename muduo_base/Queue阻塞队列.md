==阻塞队列的核心思想是：**生产者-消费者模型**==

有两种实现方式：

- （1）互斥锁+条件变量
- （2）互斥锁+信号量

# BoundedBlockingQueue.h

- 作用：
  - 有界阻塞队列：**队列长度有上限**
  - 实现为模板，传入添加的任务类型T

- 成员变量：

|               变量               |              描述              |
| :------------------------------: | :----------------------------: |
|     mutable MutexLock mutex_     |      保护阻塞队列的互斥锁      |
|       Condition notEmpty_        |           条件变量1            |
|        Condition notFull_        |           条件变量2            |
| boost::circular_buffer<T> queue_ | **循环队列**，以便空间重复利用 |

- 成员函数：

|          函数           |                 描述                 |
| :---------------------: | :----------------------------------: |
|  void put(const T& x)   | 生产者：向**非满**的queue_中添加任务 |
|        T take()         |  消费者：从**非空**的queue_中取任务  |
|   bool empty() const    |          判断queue_是否为空          |
|    bool full() const    |           判断queue_是否满           |
|   size_t size() const   |        获得queue_当前元素个数        |
| size_t capacity() const |          获得queue_最大容量          |

# BlockingQueue.h

- 作用：
  - 无界阻塞队列：队列长度无上限
  - **无界队列和有界队列的区别是：不需判断队列是否为满**

- 成员变量：

|           变量           |         描述         |
| :----------------------: | :------------------: |
| mutable MutexLock mutex_ | 保护阻塞队列的互斥锁 |
|   Condition notEmpty_    |      条件变量1       |
|   std::deque<T> queue_   |       普通队列       |

- 成员函数：

|         函数         |                描述                |
| :------------------: | :--------------------------------: |
| void put(const T& x) |     生产者：向queue_中添加任务     |
|       T take()       | 消费者：从**非空**的queue_中取任务 |
| size_t size() const  |         获得queue_当前长度         |


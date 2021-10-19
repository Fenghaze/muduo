# 日志

**在Linux下很少使用GDB来调试错误**，开发过程中，会出现编译错误和运行错误

- 开发过程中：
  - 使用日志便于调试错误，如记录errno
  - 能更好的理解程序执行逻辑
- 运行过程中：
  - 诊断系统故障并处理
  - 记录系统运行状态



# LogStream.h

- 作用：
  - 实现了**存放日志数据**的FixBuffer类
  - 实现**C++输出风格**的日志流LogStream类

- 一些成员变量

|               变量               |                             描述                             |
| :------------------------------: | :----------------------------------------------------------: |
|          Buffer buffer_          | `typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer`；日志缓冲区对象 |
| static const int kMaxNumericSize |                        默认缓冲区大小                        |

- 一些成员函数

|                    函数                    |                             描述                             |
| :----------------------------------------: | :----------------------------------------------------------: |
|      LogStream& operator<<(POD value)      | 重载<<输出运算符，参数为常见POD类型；与`std::cout<<`输出到控制台不同，`LogStream`对象使用`<<`后，会把数据保存到`_buffer`中 |
| **LogStream& operator<<(const char* str)** |          调用**_buffer.append()**添加字符串常量str           |
|   void append(const char* data, int len)   |          调用**_buffer.append()**添加指定长度的data          |
|        const Buffer& buffer() const        |                       返回buffer_对象                        |
|             void resetBuffer()             |                     调用buffer_.reset()                      |
|             void staticCheck()             |                                                              |
|           void formatInteger(T)            |                                                              |

## FixBuffer类

- 成员变量

  |       变量       |                        描述                        |
  | :--------------: | :------------------------------------------------: |
  | char data_[SIZE] |               缓冲区，SIZE为模板参数               |
  |    char* cur_    | 缓冲区指针，表示当前位置，可用于计算缓冲区实际长度 |

- 一些成员函数

  |                     函数                     |                  描述                  |
  | :------------------------------------------: | :------------------------------------: |
  | **void append(const char* buf, size_t len)** | 将buf数据复制到cur_中，并增加len个长度 |
  |                 int length()                 |          返回缓冲区字符串长度          |
  |              int avail() const               |           缓冲区当前可用空间           |
  |             void add(size_t len)             |        将cur_指针移动len个长度         |
  |                 void reset()                 |           重置cur\_为data\_            |
  |     **void setCookie(void (*cookie)())**     |                                        |
  |           const char* end() const            |            返回缓冲区尾指针            |
  |          static void cookieStart()           |                                        |
  |           static void cookieEnd()            |                                        |
  |            **void (*cookie_)()**             |                                        |



# Logging.h

作用：

- Logger日志类，暴露给用户使用的类：用户实质上**调用构造函数宏**来输出日志
- 包含枚举类型LogLevel日志级别
- 包含SourceFile日志文件类：封装和管理日志文件的两个属性（文件名，文件名长度）
- **包含Impl日志具体类**：封装日志的相关信息，**Logger的构造函数内部调用Impl的构造函数进行列表初始化**
- Logger对象构造函数的宏：方便用户调用

日志输出逻辑：

- 



## Logger类

- 成员变量

  |       变量       |       描述       |
  | :--------------: | :--------------: |
  |  enum LogLevel   |     日志级别     |
  | class SourceFile |    日志文件类    |
  |    class Impl    |    日志具体类    |
  |  **Impl impl_**  | **日志具体对象** |

- 一些成员函数

  |                    函数                     |             描述              |
  | :-----------------------------------------: | :---------------------------: |
  |   Logger(SourceFile file, int line),etc.    |           构造函数            |
  |             LogStream& stream()             | 返回impl\_.stream\_日志流对象 |
  |         static LogLevel logLevel()          |       返回全局日志级别        |
  |   static void setLogLevel(LogLevel level)   |       设置全局日志级别        |
  |      static void setOutput(OutputFunc)      |    设置输出日志的回调函数     |
  |       static void setFlush(FlushFunc)       |    设置刷新日志的回调函数     |
  | static void setTimeZone(const TimeZone& tz) |                               |



## LogLevel日志级别

| 日志级别 |                             描述                             |
| :------: | :----------------------------------------------------------: |
|  TRACE   |     指出比DEBUG粒度更细的一些信息事件（开发过程中使用）      |
|  DEBUG   |      指出细粒度消息事件来调试应用程序（开发过程中使用）      |
|   INFO   | 表明消息在粗粒度级别上突出强调应用程序的运行过程（发布之后使用，TRACE\DEBUG不会输出） |
|   WARN   |          系统能正常运行，但可能会出现潜在错误的情形          |
|  ERROR   |          指出虽然发生错误事件，但仍然不影响系统运行          |
|  FATAL   |            指出严重的错误事件，将会导致程序的退出            |



## SourceFile类



## Impl类



# AsyncLogging.h




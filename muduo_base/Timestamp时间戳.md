# Timestamp.h

- 成员变量：

|              变量               | 描述 |
| :-----------------------------: | :--: |
| int64_t microSecondsSinceEpoch_ | 微秒 |

- 一些成员函数：

|                             函数                             |              描述               |
| :----------------------------------------------------------: | :-----------------------------: |
|                  void swap(Timestamp& that)                  |    交换两个Timestamp类型的值    |
|                   string toString() const                    |     将Timestamp转换为string     |
| string toFormattedString(bool showMicroseconds = true) const | 将Timestamp转换为格式化的string |
|                      bool valid() const                      |      判断Timestamp是否合法      |
|                    static Timestamp now()                    |        返回当前系统时间         |

- 其他函数：

|                             函数                             |          描述           |
| :----------------------------------------------------------: | :---------------------: |
|     inline bool operator<(Timestamp lhs, Timestamp rhs)      |    比较Timestamp大小    |
|     inline bool operator==(Timestamp lhs, Timestamp rhs)     |  判断Timestamp是否相等  |
| inline double timeDifference(Timestamp high, Timestamp low)  | 计算两个Timestamp的差值 |
| inline Timestamp addTime(Timestamp timestamp, double seconds) |        增加时间         |


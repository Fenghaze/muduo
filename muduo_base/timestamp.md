# 继承三个基类

- muduo::copyable：空基类、标识类、值类型
- boost::equality_comparable<Timestamp>：要求实现`==`运算符
- boost::less_than_comparable<Timestamp>：要求实现`<`运算符，可自动实现其他比较运算符



# 时间起始点

1970-01-01 00:00:00


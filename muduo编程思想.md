# 基于对象编程思想

**muduo采用的是基于对象的编程思想**，不使用继承、多态

使用`boost/function.hpp`和`boost/bind.hpp`实现回调（**Reactor**）机制

> ./Thread使用了`boost/function.hpp`和`boost/bind.hpp`，可以参考其用法



EventLoop实现pool/epool模型

TCPServer接收客户端的请求

EchoServer（回射服务）定义一个TCPServer，为TCPServer注册回调函数，并在private中实现回调函数



```c++
class EchoServer
{
public:
    EchoServer()
    {
    	// 注册回调函数
        // 类成员函数的第一个参数为当前对象，在类中使用bind时，默认为 this 表示当前对象
        server_.setConnectionCallback(boost::bind(&EchoServer::onConnection, this));
        server_.setMessageCallback(boost::bind(&EchoServer::onMessage, this));
    }
private:
    // 实现回调函数
    void onConnection(){...}
    void onMessage(){...}
    
    EventLoop *loop;
    TCPServer server_;
};
```





# 面向对象编程思想

TCPServer是一个抽象类，定义抽象接口

EchoServer继承TCPServer，实现抽象接口


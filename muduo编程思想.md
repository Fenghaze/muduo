# 面向对象编程思想

核心：在类中使用了继承、多态

## 实例：封装Thread类

> 抽象类Thread.h，方法暴露给用户使用

```c++
#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
class Thread
{
public:
    Thread();
    virtual ~Thread() = 0;

    void Start();
    void Join();

    void setAutoDelete(bool autoDelete);

private:
    static void* ThreadRoutine(void* arg);
    virtual void Run() = 0;
private:
    pthread_t _threadID;
    static bool _autoDelete;
};

#endif // THREAD_H
```

> 实现Thread.cc

```c++
#include "Thread.h"
#include <iostream>

bool Thread::_autoDelete = false;

Thread::Thread()
{
    std::cout << "Thread..." << std::endl;
}

Thread::~Thread()
{
    std::cout << "~Thread..." << std::endl;
}

void Thread::Start()
{
    pthread_create(&_threadID, NULL, ThreadRoutine, this);
}

void Thread::Join()
{
    pthread_join(_threadID, NULL);
}

void* Thread::ThreadRoutine(void *arg)
{
    Thread *thread = static_cast<Thread*>(arg);
    thread->Run();
    if(Thread::_autoDelete)
    {
        thread = nullptr;
        delete thread;
    }
    return NULL;
}

void Thread::setAutoDelete(bool autoDelete)
{
    _autoDelete = autoDelete;
}
```

> 子类TestThread.cc及main

```c++
#include <iostream>
#include <unistd.h>
#include "Thread.h"
using namespace std;

class TestThread:public Thread
{
public:
    TestThread(int count):_count(count)
    {
        cout << "TestThread..." << endl;
    }

    ~TestThread()
    {
        cout << "~TestThread..." << endl;
    }
private:
    virtual void Run() override
    {
        while(_count--)
        {
            cout << "test ..." << endl;
            sleep(1);
        }
    }
private:
    int _count;
};

int main(int argc, const char** argv) 
{
    Thread *thread = new TestThread(4);
    thread->setAutoDelete(true);
    thread->Start();
    thread->Join();
    return 0;
}
```



# 基于对象编程思想

**muduo采用的是基于对象的编程思想**，不使用继承、多态

使用`boost/function.hpp`和`boost/bind.hpp`实现回调机制

boost::bind的作用是使一个函数接口转换为另一个函数接口，然后使用boost::function来接收这个新的接口

==C++11已经把boost整合到std中，直接包含functional头文件即可使用==

## 实例：适配成员函数

```c++
#include <iostream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
using namespace std;
class Foo
{
public:
    void memberFunc(double d, int i, int j)
    {
        cout << d << endl;
        cout << i << endl;
        cout << j << endl;
    }
};
int main(int argc, char const *argv[])
{
    Foo foo;
    //memberFunc接收4个参数(Foo*, double, int, int)，经过bind后，fp只接收1个参数(int)
    //_1表示占位符，fp接收的参数
    boost::function<void(int)> fp = boost::bind(&Foo::memberFunc, &foo, 0.5, _1, 10);
    fp(100);	//相当于回调了函数memberFunc(&foo, 0.5, 100, 10);
    
    boost::function<void(double, int)> fp2 = boost::bind(&Foo::memberFunc, &foo, _1, _2, 15);
    fp2(0.2, 200);
    return 0;
}
```

## 实例：封装Thread类

基于对象的编程思想不使用继承和虚函数

> Thread.h

```c++
#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
class Thread
{
public:
    typedef boost::function<void()> ThreadFunc;

public:
    explicit Thread(const ThreadFunc &threadFunc);	//传入一个线程函数来代替面向对象的多态
    ~Thread();	//去掉virtual
    void Start();
    void Join();
    void setAutoDelete(bool autoDelete);

private:
    static void* ThreadRoutine(void* arg);
    void Run();	//去掉virtual
private:
    pthread_t _threadID;
    static bool _autoDelete;
    ThreadFunc _threadFunc;		//线程函数
};
#endif // THREAD_H
```

> Thread.cc：修改了面向对象的构造函数和Run函数，其他不变

```c++
#include "Thread.h"
#include <iostream>

bool Thread::_autoDelete = false;

Thread::Thread(const ThreadFunc &threadFunc):_threadFunc(threadFunc)
{
    std::cout << "Thread..." << std::endl;
}
...
//调用传进来的线程函数
void Thread::Run()
{
    _threadFunc();
}
```

> main.cc

```c++
#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include"Thread.h"
using namespace std;

void threadFunc()
{
    cout << "this is a thread..." << endl;
}

void threadFunc2(int count)
{
    cout << "this is a thread2..." << count << endl;
}

class Foo
{
public:
    void threadFunc3()
    {
        cout << "this is a thread3..." << endl;
    }
};

int main(int argc, char const *argv[])
{
    //void()类型函数
    Thread t1(threadFunc);
    t1.setAutoDelete(true);
    t1.Start();
    t1.Join();

    //void(int)类型函数，使用bind适配接口
    boost::function<void()> threadfunc = boost::bind(&threadFunc2, 3);
    Thread t2(threadfunc);
    t2.setAutoDelete(true);
    t2.Start();
    t2.Join();

    //成员函数，使用bind适配接口
    Foo foo;
    Thread t3(boost::bind(&Foo::threadFunc3, &foo));
    //也可以传递引用
    //Thread t3(boost::bind(&Foo::threadFunc3, boost::r));
    t3.setAutoDelete(true);
    t3.Start();
    t3.Join();
    return 0;
}
```



## 实例：muduo::EchoServer类

> TcpServer是暴露给用户使用的类，类比于Thread类
>
> EchoServer是程序员实现的业务逻辑类，类比于Foo类

```c++
class EchoServer
{
public:
    //为TcpServer注册回调
    EchoServer()
    {
        server.SetConnectionCallback(boost::bind(onConnection));
        server.SetMessageCallback(boost::bind(OnMessage));
        server.SetCloseCallback(boost::bind(OnClose));
    }
    //实现的成员函数
    void OnConnection();
    void OnMessage();
    void OnClose();
private:
    TcpServer server;	//包含一个server对象
};

class TcpServer;	//定义了一个boost::function来调用注册的函数
```



# 小结

根据【实例：封装Thread类】可以看出两种编程思想的区别：

- 面向对象编程思想：
  - 定义一个抽象类，里面封装许多接口，暴露给用户使用
  - 实现不同的子类，实现继承的虚函数
  - 用户调用虚函数来实现不同的方法
- 基于对象编程思想：
  - 定义一个普通类（如Foo类、EchoServer类），里面实现许多成员函数
  - 定义一个暴露给用户使用的类（如Thread类、TcpServer类），
    - 定义一个`boost::function`来接收不同类型的函数（即用户传入不同类型的函数最终都会通过`boost::bind`转换成同一类型）
    - 在内部调用`boost::function`函数，实现不同的方法，这是另一种意义上的多态



# 不同风格实现muduo::EchoServer

- C编程风格：
  - 注册三个**全局函数**（OnConnection、OnMessage、OnClose）到网络库
  - 网络库通过**函数指针**（参数为全局函数的地址）来回调
- 面向对象编程风格：
  - EchoServer**继承**于TcpServer**（抽象类）**
  - TcpServer定义三个**函数接口**
  - 用户通过**多态**来调用这三个方法
- 基于对象编程风格：
  - EchoServer**包含**一个TcpServer对象
  - EchoServer实现这三个**成员函数**
  - EchoServer**构造函数中使用`boost::bind`为TcpServer对象注册**这三个成员函数
  - 给用户使用的TcpServer类，使用**`boost::function`来统一函数类型**，调用`boost::function`实现不同的方法


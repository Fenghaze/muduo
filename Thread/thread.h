#ifndef THREAD_H
#define THREAD_H

#include<pthread.h>
#include<boost/function.hpp>
#include<boost/bind.hpp>

class Thread
{
public:
    typedef boost::function<void ()> ThreadFunc;
    explicit Thread(const ThreadFunc &func);

    void Start();
    void Join();
    void SetAutoDelete(bool autoDelete);

private:
    static void *ThreadRoutine(void *arg);
    void Run();
    ThreadFunc func_;
    pthread_t tid_;
    bool autoDelete_;
};


#endif // THREAD_H
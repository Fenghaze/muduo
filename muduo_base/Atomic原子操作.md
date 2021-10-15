:bulb:**前提：多并发服务器要尽量减少锁操作，无锁化编程（原子操作、CAS等）能够极大提高效率**

# Atomic.h

- 成员变量：

|       变量        |    描述    |
| :---------------: | :--------: |
| volatile T value_ | 原子操作数 |

> :heavy_exclamation_mark: volatile关键字对多线程编程的重要性
>
> 作用：是一个指令关键字，确保本条指令不会因为编译器的优化而被省略，却要求每次直接读值；简单的说就是加上volatile关键字之后，**能够防止编译器对代码进行优化**
>
> 原理：当有多个线程访问同一个普通变量时，可能使用的是寄存器的备份数据，因此这会导致访问到的不是正确的数据；**因此使用volatile修饰的变量的值时，系统总是重新从它所在的内存读取数据，而不是使用保存在寄存器中的备份数据**，确保了变量的线程安全



- 成员函数：内部**调用gcc实现的原子性操作函数**

|                       函数                        |        描述        |
| :-----------------------------------------------: | :----------------: |
| \_sync_val_compare_and_swap(*ptr, oldval, newval) | 原子比较和设置操作 |
|          \_sync_fetch_and_add(*ptr, val)          |    原子自增操作    |
|        \_sync_lock_test_and_set(*ptr, val)        |    原子赋值操作    |

**注**：使用以上原子操作时，编译时需要加上`-march=cpu-type`



# 扩展-CAS无锁队列

> 摘自：https://blog.csdn.net/21cnbao/article/details/108765253

**CAS无锁队列的实质：==使用原子比较和设置操作入队==**

在要对数据进行写操作时，会先判断这个数据是否是它所期望的待修改的值，如果是则更改，如果不是，则说明有其他线程修改了这个值，此时就不能操作，因此会把期望的值更改为现在的值。

即，“==我认为V的值应该为A，如果是，那么将V的值更新为B，否则不修改并告诉V的值实际为多少==”

## 链表实现

### EnQueue版本一

- 1、每次从链表尾部进行插入
- 2、判断当前节点p是否是尾结点，如果一直是尾结点，则说明没有其他线程访问这个队列，是安全的，因此可以插入新的节点n
- 3、插入完毕后，更新尾结点

```c++
EnQueue(Q, data) //进队列
{
  //准备新加入的结点数据
   n = new node();
   n->value = data;
   n->next = NULL;
   do {
       p = Q->tail; //取链表尾指针的快照
   } while( CAS(p->next, NULL, n) != TRUE);
  //while条件注释：如果没有把结点链在尾指针上，再试
   CAS(Q->tail, p, n); //置尾结点 tail = n;
}

//如果p是尾结点，则把n插入到尾部
bool CAS(node *p, node *cur, node *n)
{
    if(p->next == NULL)
    {
        p->next = n;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
```

会看到，为什么我们的“置尾结点”的操作（第11行）不判断是否成功，因为：

- 1、如果有一个线程T1，它的while中的CAS如果成功的话，那么其它所有的 随后线程的CAS都会失败，然后就会再循环
- 2、此时，如果T1 线程还没有更新tail指针，其它的线程继续失败，因为`tail->next`不是NULL了
- 3、直到T1线程更新完 `tail` 指针，于是其它的线程中的某个线程就可以得到新的 `tail` 指针，继续往下走了
- 4、所以，只要线程能从 while 循环中退出来，意味着，它已经“独占”了，`tail` 指针必然可以被更新

这里有一个潜在的问题——**如果T1线程在用CAS更新tail指针的之前，线程停掉或是挂掉了，那么其它线程就进入死循环了**。下面是改良版v1

### EnQueue改良版v1

改进：让每个线程，自己fetch指针 `p` 到链表尾。

```c++
EnQueue(Q, data) //进队列改良版v1
{
   //准备新加入的结点数据
   n = new node();
   n->value = data;
   n->next = NULL;
   //保存当前尾结点
   p = Q->tail;
   oldp = p;
   do {
       while (p->next != NULL)	//每个线程自己找到尾结点
           p = p->next;
   } while( CAS(p->next, NULL, n) != TRUE); //如果没有把结点链在尾上，再试
   CAS(Q->tail, oldp, n); //置尾结点
}
```

但是这样的fetch会很影响性能；而且，如果一个线程不断EnQueue，会导致所有的其它线程都去 fetch 他们的 `p` 指针到队尾，能不能不要所有的线程都干同一个事？这样可以节省整体的时间？下面是改良版v2

### EnQueue改良版v2

改进：直接 fetch `Q->tail` 到队尾

因为，所有的线程都共享着 Q->tail，所以一旦有人动了它后，相当于其它的线程也跟着动了

```c++
EnQueue(Q, data) //进队列改良版 v2
{
   n = new node();
   n->value = data;
   n->next = NULL;
   while(TRUE) {
      //先取一下尾指针和尾指针的next(NULL)
       tail = Q->tail;
       next = tail->next;
      //如果尾指针已经被移动了，则重新开始
       if ( tail != Q->tail ) continue;
      //如果尾指针的 next 不为NULL，则 fetch 全局尾指针到next
       if ( next != NULL ) {
           CAS(Q->tail, tail, next);
           continue;
       }
      //如果加入结点成功，则退出
       if ( CAS(tail->next, next, n) == TRUE ) break;
   }
   CAS(Q->tail, tail, n); //置尾结点
}
```

### DeQueue

```c++
DeQueue(Q) //出队列
{
   do{
       p = Q->head;
       if (p->next == NULL){
           return ERR_EMPTY_QUEUE;
       }
   while( CAS(Q->head, p, p->next) != TRUE );
   return p->next->value;
}
```

### DeQueue改良版

```c++
DeQueue(Q) //出队列，改进版
{
   while(TRUE) {
      //取出头指针，尾指针，和第一个元素的指针
       head = Q->head;
       tail = Q->tail;
       next = head->next;
      // Q->head 指针已移动，重新取 head指针
       if ( head != Q->head ) continue;
      // 如果是空队列
       if ( head == tail && next == NULL ) {
           return ERR_EMPTY_QUEUE;
       }
      //如果 tail 指针落后了
       if ( head == tail && next == NULL ) {
           CAS(Q->tail, tail, next);
           continue;
       }
      //移动 head 指针成功后，取出数据
       if ( CAS( Q->head, head, next) == TRUE){
           value = next->value;
           break;
       }
   }
   free(head); //释放老的dummy结点
   return value;
}
```



## 数组实现


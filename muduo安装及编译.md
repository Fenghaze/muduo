- 源码地址：https://github.com/chenshuo/muduo/releases
- 使用Windows Source Insights学习源码
- 使用Linux执行示例
- 使用Graphz+Doxygen生成API

# Windows

- 安装Source Insights 4.0
- New Project->设置项目保存目录和项目名->修改源码地址->add Trees
- Options->File Type Options->C/C++ Source File->添加`*.cc;*`，**可以让Source Insights识别`.cc`文件**
- Project->Open Project->打开Source Insights自带的Base项目
- 打开utils.em文件，在文件末尾追加switch_cpp_hpp宏
- 重新打开Source Insights，Options->Key Assignments->找到Macro:switch_cpp_hpp->Assign New Key->设置Ctrl+B为切换快捷键->OK，**可以使用Ctrl+B对头文件和源文件进行切换**
- **查看文件中函数调用关系**：View->Panels->Relation Window->齿轮设置->For Functions->Calls

# Linux

- 安装所需的库
  - CMAKE：`sudo apt-get install cmake`
  - BOOST：`sudo apt-get install libboost-dev libboost-test-dev`
- 解压源码
- `./build.sh -j2`，生成build/release-cpp11：bin目录下包含一些可执行示例
- `./build.sh install`，生成build/release-install-cpp11：包含include、lib，系统环境设置这两个目录即可在编写代码时引入头文件

# 编译运行

假设muduo是存放在`/home/zhl/file`目录下的

- 方法一：==设置系统环境变量==

```shell
sudo ln -s ~/file/build/release-install-cpp11/include/muduo/ /usr/include/muduo
sudo ln -s ~/file/build/release-install-cpp11/lib/*.a /usr/lib/
```

- 方法二：
  - 把`release-install-cpp11/include/muduo/`复制到`/usr/include/muduo`
  - 把`release-install-cpp11/lib/*.a`复制到`/urs/lib`

- 设置编译选项（以/examples/echo为例）

```shell
g++ main.cc echo.cc -o  echo -lmuduo_net -lmuduo_base -lpthread #-l是指定静态链接库
```

- 或编写makefile文件

```makefile
CC=g++
CFLAGS+=-g -Wall -lmuduo_net -lmuduo_base -lpthread
OBJS=echo.cc main.cc

echo: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -rf echo
```

- 或编写CMakeLists.txt文件

```cmake
#cmake的版本要求
cmake_minimum_required(VERSION 3.10)

#工程的名称
project(echo)

#生成可执行文件 后面是依赖的源文件
add_executable(echo echo.cc)

#添加静态链接库 muduo_base muduo_net
target_link_libraries(echo muduo_base muduo_net)
```

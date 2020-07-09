# 安装依赖

- CMAKE：`sudo apt-get install cmake`
- BOOST：`sudo apt-get install libboost-dev libboost-test-dev`



# 安装muduo

```shell
cd muduo/
./build.sh -j2
./build.sh install
```

最后会生成2个文件夹

- ./build：release版
  - /release-install-cpp11：包含include、lib，系统环境设置这两个目录即可在编写代码时引入头文件
- ./muduo：源代码、examples存放位置



# 编译运行examples

muduo是存放在`/home/zhl/file`目录下的

- ==设置系统环境变量==

```shell
sudo ln -s ~/file/build/release-install-cpp11/include/muduo/ /usr/include/muduo
sudo ln -s ~/file/build/release-install-cpp11/lib/*.a /usr/lib/
```

- 设置编译选项（以/examples/echo为例）

```shell
g++ main.cc echo.cc -o  echo -lmuduo_net -lmuduo_base -lpthread
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


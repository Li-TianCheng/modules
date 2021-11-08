# C++基础模块
### 实现模块
1. [事件系统](#事件系统)
2. [pthread封装](https://github.com/Li-TianCheng/LibPthread)
3. [对象池](#对象池)
4. [任务系统](#任务系统)
5. [时间系统](#时间系统)
### 设计思路
##### 总体思路
* 事件系统用于模块间的信息交换
* 对象池用于模块间的共享对象的管理
* 时间系统用于定时和循环定时
* 任务系统为自适应线程池
* 不同模块均运行在不同的线程中
##### 事件系统
* 将事件注册到事件系统中
* 含有一个cycle用于处理事件
* 在对象池中申请一个Event,并向目标事件系统添加该事件
* 事件系统会在cycle中从eventQueue取出事件,并执行相应handle
##### [pthread封装](https://github.com/Li-TianCheng/LibPthread)
##### 对象池
* 对[内存池](https://github.com/Li-TianCheng/MemPool) 的简单封装,返回共享指针
##### 资源系统
* 继承事件系统
* 可对注册的资源进行increase,decrease,checkout操作
##### 任务系统
* 继承Resource,注册在资源系统中
* 当taskQueue满时,会发送一个扩容事件
* 向资源系统添加一个循环定时事件,当事件触发时,进行缩容
* 扩容策略：当线程池中线程全都在执行任务时,扩容initNum个,但threadNum不能大于MaxNum
* 缩容策略：当taskQueue为空时,缩容initNum个,但threadNum不能小于initNum,且只取消阻塞的线程
##### 时间系统
* 继承事件系统
* 可以添加定时事件和循环定时事件
* 利用时间轮的方式进行定时
* 利用epoll进行定时,来推动时间轮
* 当相应的定时器过期时,会向事件添加者发送事件
##### 配置系统
* 全局唯一配置,读取指定路径json文件
##### 日志系统
* 异步日志系统
* 继承Resource,注册在资源系统中
* 资源系统定时将日志输出
##### my_sql
* 继承Resource,注册在资源系统中
* 含有连接池
* 资源系统对连接池进行扩缩容
##### redis
* 继承Resource,注册在资源系统中
* 含有连接池
* 资源系统对连接池进行扩缩容
##### net
* TCP网络库
* EpollTask承载连接的epoll,继承事件系统,可接受自定义事件和epoll事件,运行在任务系统中
* listener使用epoll实现的监听器,可监听多个端口,可动态监听关闭端口,继承事件系统
* listener可将连接动态的分配到EpollTask上,保持负载均衡,并可对EpollTask进行扩缩容
* 可自定义Session继承TcpSession,利用模板参数生成TCP服务TcpServer&lt;Session&gt; ,将TcpServer&lt;Session&gt;注册到listener,并开启监听即可实现自定义Tcp服务
##### http
* 根据网络库,实现的http服务
##### utils
* 常用小工具
* 序列化
* 进度条
* 字符串工具
##### 启动时图示
![](https://github.com/Li-TianCheng/modules/blob/main/modules.png)
### 用法
1. 在main开始时调用modules::init使各个模块初始化
2. 在main结束时调用modules::close使各个模块关闭
* 调用TimeSystem::receiveEvent(),即可添加时间事件，时间间隔为struct Time(h, m, s, ms)
* 调用TaskSystem::addTask(),即可添加任务

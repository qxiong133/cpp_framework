
代码存放目录说明:
third_party存放的是第三方库,include存放头文件,lib存放第三方库.例如:poco,boost,zeromq,msgpack等
src目录存放源代码,
    1.gateway_server 网关代码
    2.route 路由分发
    3.workers   所有的workers服务,目前包括simple_worker和pvp_worker
    4.module    数据库和redis接口
    5.utils 通用的小工具,一些方便开发的类和函数

build目录cmake的编译目录
    1.使用cmake ..生成makefile
    2.生成bin目录,存放make生成的二进制文件
    3.make install会把文件复制到output目录,同时拷贝conf，log文件夹到output目录下面

conf目录存放所有的配置文件，在编译的时候会拷贝到build下面的output子目录下面,每个工程的配置文件统一存放在这个文件夹下面

log目录存放所有的日志文件，在编译的时候会拷贝到build下面的output个子目录下面

script目录存放脚本文件,例如服务的启动脚本,在编译的时候会拷贝到build下面的output个子目录下面

   

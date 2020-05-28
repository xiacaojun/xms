
###  XMS 微服务系统包含 API 网关，日志服务，配置服务，注册服务，服务管理中心， 通信组件 SDK，数据库读写 SDK。 除了 SDK 类的模块其他各模块应具备进行独立集群部署的能力。 XMS 系统应提供 C/S 的系统管理界面，进行配置管理、服务状态监控、日志查询功能。 

# 项目参与人员和贡献
## 夏曹俊 

<table border=1>
<tr>
<th>name</th>
<th>贡献</th>
<th>邮箱</th>
</tr>
<tr>
<td>
夏曹俊
</td>
<td>
1 项目运维<br>
2 XPlatform通信库开发
</td>
<td>
xiacaojun@qq.com
</td>
    
文轩萱	wenxuanxuanJoey	2499920330@qq.com
刘路路	ysrs 	lu_lu_liu@163.com  
周天文	T1mzhou	354598311@qq.com
吴晓明	wuxm04@126.com	wuxm04@126.com
韩新乐	hanxinle	hanxloop@foxmail.com

    
</tr>
    
    
    <tr>
<td>
于海江
</td>
<td>
参与xmservice开源库开发
</td>
<td>
798737767@qq.com
</td>
</tr>

</table>



# Windows编译环境准备
安装VS2017社区版本和QT5.9版本
# Linux编译环境准备
ubuntu 18.04.02 x64
##?公共的工具
    apt-get?install?perl?g++?make?automake?libtool?unzip git
# 依赖库编译安装
## zlib（protobuf，libevent依赖）（压缩）
    tar -xvf zlib-1.2.11.tar.gz
    cd zlib-1.2.11/
    ./configure
    make -j32
    make install
    # 安装在 /usr/local/include/ /usr/local/lib 目录下

## openssl （libevent依赖）（安全加密）
    tar -xvf openssl-1.1.1.tar.gz
    cd openssl-1.1.1/
    ./config
    make -j32
    make install
    # openssl 命令行 /usr/local/bin
    #配置安装在 /usr/local/ssl 
    #头文件/usr/local/include/openssl
    #so库文件/usr/local/lib
## protobuf（通信协议）
    unzip protobuf-all-3.8.0.zip
    cd protobuf-3.8.0/
    ./configure
    make -j32
    make install
    #安装在 /usr/local/include/google/protobuf 
    # protoc /usr/local/bin
    # so库文件 /usr/local/lib
## libevent （网络通信）
    unzip libevent-master.zip
    ./autogen.sh
    ./configure
    make -j32
    make install
    #安装在 /usr/local/lib /usr/local/include


# XMS 系统安装

## 安装数据库服务器（Linux）
### 安装数据库客户端库
    apt-get install libmysqlclient-dev
### 安装数据库服务端
    sudo apt-get install mysql-server
### 配置用户名密码
    /etc/mysql/debian.cnf文件，在这个文件中有系统默认给我们分配的用户名和密码
    mysql -u debian-sys-maint -p 
    set password for 'root'@'localhost' = password('123456')
## LXMysql 库安装
    apt-get install libmysqlclient-dev
    cd /root/xms/src/LXMysql
    make -j32
    make install
    # 安装在 /usr/lib/libLXMysql.so
## XPlatform通信库安装
    cd ../xplatform
    # 生成proto对应的c++代码
    make proto  
    make -j32
    make install
    # 安装到 /usr/lib/libxcom.so
## XRC注册中心安装
依赖XPlatform，并且会连接XLOG日志中心
注册中心服务端安装
    cd ../register_server
    make -j32
    make install
## 注册中心客户端
    cd ../register_client
    make -j32
    make install
## XLOG日志中心安装
    #依赖XPlatform，LXMysql register_client 会连接XRC注册微服务
    cd ../xlog/
    make -j32
    make install
## XCC配置中心安装
    # 依赖LXMysql xplatform register_client 会连接XRC注册微服务
### 配置中心微服务
    cd ../config_server
    make -j32
    make install
### 配置中心客户端
    cd ../config_client
    make -j32
    make install
## XAUTH 鉴权中心安装
    # 依赖LXMysql xplatform register_client 
    # 通过register_client 连接XRC注册微服务
    # 通过 config_client 获取配置
    cd ../xauth
    # 编译xauth微服务
    make -j32  
    # 编译xauth客户端
    make libxauth.so
    make install
## XAG网关安装
    # 依赖xplatform register_client config_client
    # 通过register_client获取全部可用微服务列表
    # 通过 config_client 获取网关配置
    make -j32
    make install

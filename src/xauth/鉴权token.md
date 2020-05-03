# 鉴权TOKEN
## token的生成策略
## 用户名 ，有效期
## 获取token
### 通过用户名或者密码获取
### 通过快过期token获取新的token
## 验证token
## 鉴权中心初始化
- 创建表
- 添加root用户
## 客户端
- 使用SSL通信登录发送用户名和md5编码后的密码
- 获取token后本地存储，每次发送消息前验证token有效期 （有效期设为30分钟）
- 有效期快过期前（提前一分钟）重新获取token
## 服务端
- 接收用户的用户名和md5编码后密码验证
- 验证成功生成token token暂定只是唯一标识，写入数据库关联用户表 
- token 表
- 如果需求中，一个用户只能有一个客户端登陆，可以直接放在用户表中。
- 每次清理超时的token
    xms_token
        id int  
        token varchar(36) 
        expired_time int 过期时间戳 
- LXMysql 是静态编译的，项目预处理器定义要加 STATIC

## 时间戳
    使用c语言的 time函数
    time_t t1 = time(0); 
## UUID()
UUID(): 450e1572-a298-11e4-aa3c-08002735e4a4
本身 UUID 是 32 位，因为 MySQL 生成的 UUID 有四个中划线，所以在 utf8 字符集里，长度为 36 位。



# gsf_sample
gsf distributed sample

 1. 打开gsf_sample/CMakeLists.txt， 修改GSF_PATH中的路径 到你gsf项目工程的路径。
 2. 打开gsf_sample/build 执行cmd cmake.. 构建工程
 3. 使用多个gate
  > * 复制distributed_gate.exe + script/cfg.lua 到另外一个目录
  > * 修改cfg.lua 配置驱动文件（更改以下两个id
  
> * distributed_id 进程在集群中的id（唯一，我目前用的端口号模拟
> * acceptor_port 进程的服务器id

```lua

-- 配置中的modules 描述当前进程需要注册进负载均衡的 Module
-- "GateLoginModule" module名称
-- 0 module 的特征id， 默认为0 （名称一样，但装入的数据不一样的时候可以用特征来切分。 例如scene module

local modules = { {"GateLoginModule", 0}, }

```

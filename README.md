sirius
============================

<p align="center">
    <a href="https://github.com/gottingen/kumo-search/blob/master/docs/product.md"><font face="黑体" color=#0099fc size=4>Documentation</font></a>
    <a></a> <font face="黑体" color=#6628ff size=4> &nbsp;.&nbsp; </font></a>
    <a href="https://github.com/gottingen/kumo-search/blob/master/docs/product.md"><font face="黑体" color=#0099fc size=4>文档</font></a>
    <a></a> <font face="黑体" color=#6628ff size=4> &nbsp;.&nbsp; </font></a>
    <a href="CONTRIBUTORS.md"><font face="黑体" color=#0099fc size=4>Contributors</font></a>
    <a></a> <font face="黑体" color=#6628ff size=4> &nbsp;.&nbsp; </font></a>
    <a href="NEWS.md"><font face="黑体" color=#0099fc size=4>Road Map</font></a>
</p>

sirius 是 `EA` 服务架构中的元数据服务，提供服务发现，全局统一时钟，全局唯一ID，配置管理功能。sirius是基于raft协议实现的分布式一致性服务，
支持多副本，自动选主，自动故障转移，自动数据同步等功能。

# 编译与安装

sirius 依赖于 `carbin` 服务，编译安装前请确保 `carbin` 服务已经安装并启动。`carbin` 服务的安装请参见[carbin](https://carbin.readthedocs.io/zh_CN/latest/)
sirius 服务的安装有两种方式，一种是通过源码安装，另一种是通过二进制安装。
## 源码安装
源码安装依赖环境参见[环境依赖](https://github.com/gottingen/kumo-search/blob/master/docs/inf/inf.md)
```shell
    carbin install gottingen/sirius --prefix=/opt/EA/inf
```

## 二进制安装

二进制默认提供基于基础环境的的安装包，建议使用源码安装，兼容性更好。部署过程中，只依赖基础环境，编译过程中的依赖会以静态库的方式打包到二进制包中。
用户在自己的环境上进行编译和分发安装。

编译sirius：

```shell
    git clone https://github.com/gottingen/sirius.git
    git checkout xxx #tag or branch
    cd sirius
    carbin install --prefix=/opt/EA/inf
    mkdir build
    cd build
    cmake ..
    make -j 4
    make package
```
${sirius_dir}/build目录下，package目录下有编译好的二进制包，如果是centos7系统，生成 `rpm`和`sh`包，如果是ubuntu系统，生成`deb`和`sh`包。
值得注意的是，`rpm`包在编译过程中，如果使用的是`devtoolset-*`编译的，那么在安装的时候，需要使用`devtoolset-*`的环境来安装，否则会出现依赖问题。
并且安装过程中，有可能会出现libstdc++.so.6文件缺失的问题，安装`rpm`包的时候，需要加上`--nodeps`参数。

```shell
    rpm -ivh --nodeps sirius-1.0.0-1.el7.x86_64.rpm
```

# 使用文档

sirius的使用文档请参见[使用文档](https://github.com/gottingen/kumo-search/blob/master/docs/product.md)

# 贡献者

sirius的贡献者请参见[贡献者](CONTRIBUTORS.md)

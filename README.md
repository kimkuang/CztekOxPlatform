# CztekOxPlatform

## 项目介绍

[![深圳市辰卓科技有限公司](http://www.cztek.cn/image/cztek_logo_1.png)](http://www.cztek.cn)成立于2013年5月，是一家专注于嵌入式系统产品、工业检测与测试系统产品研发和服务的综合性创新型国家级高新技术企业。<br>
CztekOxPlatform是深圳市辰卓科技有限公司维护的一个摄像头模组测试仪软件开源项目，旨在与摄像头模组提供商一起，提升测试效率、提高产品测试良率。

## 产品简介

### CZCM51X系列 MIPI 1.5G摄像头模组测试仪
<p align="center">
  <img alt="CZCM52X" src="http://www.cztek.cn/product/CZCM517/CZCM517a.png">  
</p>

### CZCM52X系列 双摄像头模组测试仪
<p align="center">
  <img alt="CZCM52X" src="http://www.cztek.cn/product/CZCM527/CZCM527a.png">
</p>


## 软件框架

CztekOxPlatform系统平台，行业内首创通过在基于ARM的单板电脑上直接采集模组图像数据，并直接在测试仪内完成测试，不用将大量的图像数据上传到PC再处理。整个系统减小了系统复杂度，降低了大量图像数据上传带来的系统开销和不稳定性，测试效率更高，在自动化测试机台上运行更稳定。<br>
分布式的体系架构，有效降低了高度依赖电脑计算的负荷，在各单板电脑上即完成了测试，测试数据通过网络能全部上报到制造执行系统。


## 安装教程

1. 克隆项目；
2. 进入目录02OpenSource/czcmti，执行自动化编译脚本build_all.sh；
3. 执行02OpenSource/czcmti/Release目录下生成的自动安装包*.run文件。

## 使用说明

请参阅《[辰卓科技模组测试仪四焦段机台AF水平烧录站位使用手册通用版V1.0.pdf](/02OpenSource/Doc/使用手册/辰卓科技模组测试仪四焦段机台AF水平烧录站位使用手册通用版V1.0.pdf)》。

## 参与贡献

1. Fork 本项目
2. 新建 Feat_CztekOxPlatform 分支
3. 提交代码
4. 新建 Pull Request


## 特性

1. 全球首创基于Ubuntu系统的ARM单板电脑（Single Board PC）的模组测试仪，省去图像传输延时，最快帧率输出，提升UPH。
2. GPU加速图像处理方案。
3. 可同时单、双和三通道测试，API兼容串行和并行两种设计模式。
4. 支持电脑侧和设备侧协同工作的分布式架构。
5. 与终端平台一致的硬件平台，测试结果一致性更高。

## License
Copyright (c) Shenzhen CZTEK Co., Ltd. All rights reserved.

Licensed under the [Apache-2.0](LICENSE) License.

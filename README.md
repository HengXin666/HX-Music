# HX-Music

> [!TIP]
> 仍在绝赞开发中...

- C++ QT/QML 客户端: [HX-Music-Client/README.md](HX-Music-Client/README.md)

- C++ 服务端: [HX-Music-Server/REAMDE.md](HX-Music-Server/REAMDE.md)

- Python 日语注音 并且为 双行 卡拉ok ASS => [pyTool/README.md](pyTool/README.md)

---

已知问题:

- [x] 玄学的断开的管道, 大部分时候没事. 但是有时候会触发. 目前不知道如何稳定复现. (原因: [HXLibs: Commit 3d1466b](https://github.com/HengXin666/HXLibs/commit/3d1466bd9e61c0708e9a93893d650ebe02157482))

- [x] 上传音乐无法及时刷新到界面上. 需要手动刷新... (需要同步一下)

- [ ] 玄学的随机的 ass right 预处理计算错误. (使用其他方法临时规避了... 不过感觉有隐患...)
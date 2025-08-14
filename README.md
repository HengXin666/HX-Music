# HX-Music

重构!!!

```cpp
1. 树状播放列表, 修改为纯播放列表
2. 使用QML, 并且支持 ass 全屏

- ASS 设置界面

    - 应该支持两种模式: 全屏 || 截屏

    - 全屏模式: 直接获取到当前屏幕的大小, 然后直接全屏渲染

    - 截屏模式: 通过数学计算 图片的大小位置, 然后裁剪无用的
        - 实现1: 需要分离弹幕的, 存在上下分割, 需要合并
        1. 计算竖直方向的中心线, 上方坐标为上边, 下方坐标为下边. 直接分为2组
        2. 分别计算 x, y 上的矩形范围, 最后通过偏移量合并渲染

        问题: 抖动
            1. 因为存在位图等特效渲染, 导致 y 的偏移变化大, 导致字体在y方向抖动
            => 解决方案, 分别渲染: 带布局计算的 和 不带布局的
                带布局的计算到布局, 不带布局的直接覆盖重合在布局上面, 不参与布局.
            2. 发现问题依旧: 原来, 歌词的上下方向的动画也会导致 y 的预留, 从而导致y偏移抖动
            => 解决方案, 依据是过滤. 发现问题: 超难过滤! 因为动画是复合命令, 比如: (而且样本很小, 不能保证也适配其他ass文件)

// ass
Dialogue: 1,0:00:57.05,0:00:57.07,OP JP 4,,0,0,0,fx,{\an5\pos(526,37)\blur1\fscx130\fscy130}イ
Dialogue: 1,0:01:12.29,0:01:17.54,OP CN 2,,0,0,0,fx,{\an5\pos(577,690)\blur1\fad(0,350)}的
Dialogue: 1,0:01:04.95,0:01:04.96,OP JP 6,,0,0,0,fx,{\an5\blur1\clip(755,42,815,47)\pos(782,37)}と

    - 全新的解决方案!
        - 直接扫描全部, 确定好完整的坐标范围 (矩形)
        - 直接在渲染出来的部分上, 裁剪矩形部分, 到真正的输出图片中
        - 这样完全不会有y偏移的影响, 也不需要过滤ass! 直接预处理了
        - 只需要按照 60 帧每秒, 然后采集整个歌词即可, 日后可以生成缓存 (现在想预留缓存接口)
        - 然后给出配置: 上下的边距 和 中间空白边距 即可
    - 然后就又出现问题了, 因为是裁剪矩形, 导致左边会有空白, 因此虽然这很稳定, 但是会对齐不上
        - 本来是打算做一个图片裁剪的, 但是还不如直接渲染之前就fk掉, 反正左右都会抖动了 =-=
        - 修改一下代码, 试试效果
        - 效果还行! 几乎没有抖动, 但是如果动画出现左右偏移, 越界了本身歌词的位置, 就会导致水平抖动
        - 解决方案: 
            1. 直接分离歌词 感觉不彻底
            2. 加左右边距!  实惠还管饱, 注意要左右一起加, 否则不居中... 不对, 我根本不知道这是什么, 不知道什么时候加啊
            退回方案 1. (@todo)

架构歌曲存储: 多歌单, 多歌曲.
{
    playlistId: "歌单id"
    playlistDescription: "歌单描述"
    songList: [
        "歌单内容",
        "url",
        "file://" // path
    ]
}

什么时候加载歌单?
    - 启动时候, 全局配置得到歌单
    - 然后进入歌单页面加载歌单, 然后得到歌单列表, 结合索引, 得到上一次播放的歌曲

界面美化:
    - 无边窗口

歌单列表:
    - 拖拽加入, 加入是指定位置
    - 歌曲相同, 则应该是删除原本的歌曲, 然后插入新的歌曲到拖拽的地方

本地还需要记录的配置:
    - 歌曲
        - 退出之前所在的歌单
        - 歌单的第几项索引
        - 歌曲的播放时间
    - 歌词
        - 歌词偏移量
        - 歌词窗口是否打开
        - 歌词窗口的位置、大小

发现: 依据不能很好的支持歌词窗口记忆, 因为需要 set 坐标.

所以最后的解决方案就是! 全屏! 然后裁剪信号. 这样我们仅需要保证多窗口情况下正确即可.
```

## 环境要求

- QT 6.9.1 (开发环境)

- 窗口系统 Wayland (开发环境)

- KDE Plasma 6.4.3 (开发环境)

- Arch Linux 内核 6.15.9 (开发环境)

> [!TIP]
> Wayland 特殊适配:
>
> 因为默认不支持顶置, 故应该在 设置 > 窗口管理 > 规则 > 新建规则 > 添加两个规则:
>   1. 匹配窗口标题 `[Wayland置顶]`
>   2. 强制的置顶窗口
>
> 才可以置顶窗口

## 依赖安装
### 1. taglib
> [!TIP]
> 大多数 Linux 发行版都预装了 TagLib

[[Github] taglib](https://github.com/taglib/taglib)

```sh
sudo pacman -S taglib
```

### 2. libass

[[Github] libass](https://github.com/libass/libass)

```sh
sudo pacman -S libass
```

<!-- 无需安装, 发现使用QWindow再套用QWidget就好了 awa?!
### 3. Wayland 兼容

```sh
sudo pacman -S gtk3
sudo pacman -S gtk-layer-shell
sudo pacman -S kwindowsystem
```

> [!TIP]
> 由于 Wayland 协议的原因, qt很难做到移动窗口, 草怎么好多都不行...
>
> QT你滴什么滴干活?!
>
> 再也不相信任何 QT + Wayland 的所谓解决方案了, 纯纯浪费时间.

可以使用xcb, 这样可以move, 但是! 之前某些为了 Wayland 设计的东西就需要重构了, 因此 fuck you 了您嘞

```cpp
qputenv("QT_QPA_PLATFORM", "xcb");
```

-->

## Win系统配置

1. 安装`vcpkg`, 下载`taglib`和`libass`

2. 添加`.vscode/settings.json`写入:

```json
// .vscode/settings.json
{
  "cmake.configureSettings": {
    // 你的 vcpkg 的对应路径
    "CMAKE_TOOLCHAIN_FILE": "D:/MyApp/vcpkg/scripts/buildsystems/vcpkg.cmake",
    "VCPKG_TARGET_TRIPLET": "x64-windows"
  }
}
```

3. 下载4个qt插件 (vscode)

4. 修改根cmake:

```cmake
if(WIN32)
    # Qt编译器路径设置
    set(CMAKE_PREFIX_PATH "D:/MyApp/Qt/6.8.2/msvc2022_64")

    # vcpkg下载的包的路径
    set(LIB_ROOT "D:/MyApp/vcpkg/installed/x64-windows")
endif()
```

5. 尝试编译, 如果不行, 请喷qt、vcpkg、msvc, 以及疯狂喷win系统, 然后屁颠屁颠去使用linux!

> [!ERROR]
> win下, taglib无法解析到专辑图片, 始终无法打开文件... 而文件描述什么的却可以...
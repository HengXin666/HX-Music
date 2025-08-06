# HX-Music

重构!!!

```cpp
1. 树状播放列表, 修改为纯播放列表
2. 使用QML, 并且支持 ass 全屏
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
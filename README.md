# HX-Music

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

-->
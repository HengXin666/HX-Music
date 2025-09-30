# HX-Music 服务端

## 一、Docker 部署
### 1.1 DockerFile

- 请见: [dockle/REAMDE.md](./docker/REAMDE.md)

### 1.2 Docker-compose

下方是 `docker-compose.yaml`, 按需修改. 一般仅需要修改带注释的地方

```yaml
services:
  hx-music-server:
    image: hengxin666/hx-music-server:latest
    container_name: hx-music-server
    ports:
      # 端口映射
      - "28205:28205"
    volumes:
      # 文件存放路径, 包含数据库、音乐文件、字幕文件
      - ./data:/loli/HX-Music/data
    restart: always
    environment:
      - AEGISUB_HOME=/usr/share/aegisub
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:28205/"]
      interval: 30s
      timeout: 5s
      retries: 3
    security_opt:
      - seccomp:unconfined
    cap_add:
      - SYS_ADMIN
      - SYS_RESOURCE
      - NET_ADMIN
```

## 二、本地构建

> [!TIP]
> 比较麻烦, 但是如果需要进行开发, 那这是必须滴.

- 推荐使用 Arch Linux

1. 安装需要的包

```sh
pacman -Syu git base-devel sudo wget unzip cmake clang taglib python python-pip pybind11 openssl
```

2. 安装 yay

```sh
git clone https://aur.archlinux.org/yay.git \
    && cd yay \
    && makepkg -sri --needed --noconfirm \
    && cd .. && rm -rf yay .cache
```

3. 使用 yay 安装 Aegisub

```sh
yay -S --noconfirm aegisub
```

4. 克隆 HX-Music

```sh
git clone https://github.com/HengXin666/HX-Music.git HX-Music
```

5. 复制 Lua 脚本到 Aegisub

> 注意路径

```sh
mkdir -p /usr/share/aegisub/automation/autoload/ \
    && sudo cp HX-Music/pyTool/lua/set-karaoke-style.lua /usr/share/aegisub/automation/autoload/
```

6. 安装 uv 并且配置虚拟环境

在 HX-Music/pyTool 中

```sh
# 安装 uv
curl -LsSf https://astral.sh/uv/install.sh | sh

# 同步 环境
uv sync
```

7. 测试使用

```sh
uv run main.py
```

会下载一些库的依赖语素等, 然后会输出一些注音结果.

8. 构建 HX-Music-Server

```sh
# 下载本项目的依赖git库
git submodule update --init --recursive

# 在项目根目录: HX-Music/
mkdir build
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CLIENT=OFF \
    -DBUILD_SERVER=ON

cmake --build . --config Release --target HX-Music-Server --
```

9. 启动程序

```sh
# 1. 先启动虚拟环境, 在之前安装 uv 环境处 (HX-Music/pyTool)
source ./.venv/bin/activate

# 2. 启动程序
./HX-Music-Server
```
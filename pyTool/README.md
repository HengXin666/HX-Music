# Ass 注音工具
## 一、安装依赖
### 1.1 Python 依赖 (基于 UV)

0. 使用 UV 管理 (下载UV)

```bash
# On macOS and Linux.
curl -LsSf https://astral.sh/uv/install.sh | sh

# On Windows.
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

# With pip.
pip install uv
```

1. 创建环境

```bash
# 注意路径, 推荐就在 ./pyTool
uv init
```

2. 同步依赖

```bash
uv sync
```

3. 启动

```bash
uv run main.py
```

### 1.2 设置`英语-片假名`词库 (可选)

> [!TIP]
> 本程序已经自带, 本步骤可跳过

关于英语单词映射, 优先使用词典; 您可以手动安装:

```bash
# 克隆仓库
git clone https://github.com/Patchethium/e2k.git
cd e2k/vendor

# 下载数据
curl -O https://kaikki.org/dictionary/downloads/ja/ja-extract.jsonl.gz
curl -O http://ftp.edrdg.org/pub/Nihongo/edict2.gz

# 解压
gzip -d ja-extract.jsonl.gz
gzip -d edict2.gz

# 在 e2k 目录
# 生成 katakana_dict.jsonl
python extract.py

# 然后把 katakana_dict.jsonl 复制到 pyTool/src/data
```

词典可能有多个对照, 本程序默认取第一个作为片假名.

如果字典没有, 则通过语素转换到片假名.

如果您认为转换效果不好, 可以自行在 [`katakana_dict.jsonl`](./src/data/katakana_dict.jsonl) 中添加自己的映射.

### 1.3 安装 Aegisub 和 Aegisub-cli

1. 程序需要使用 Aegisub 以应用 `自动化卡拉ok模板.lua`.

因此需要安装: [Aegisub](https://github.com/TypesettingTools/Aegisub/releases)

2. Aegisub 仅提供了运行GUI 和 Lua 环境, 但是并不能直接通过命令行调用.

因此, 为了从 Python 中调用 Aegisub-Cli 以使用 `自动化卡拉ok模板.lua` 处理 Ass.

需要安装: [aegisub-cli](https://github.com/HengXin666/aegisub-cli)

特别的, 如果是 Windows 用户, 可以直接下载 `aegisub-cli.exe` 到 `pyTool/bin` 目录中

如果 Arch Linux 用户需要手动构建的话. 可以直接 克隆 上面链接的库. 我调整了项目结构以让 `meson` 可以成功构建.

并且, 把项目C++版本升级到 **C++20**, 以适配第三方库依赖的模板需要的特性. 并且调整了 Boost 头文件, 让它不会编译报错.

其他的Linux发行版, 我没有也不想测试.

### 1.4 安装依赖的 Lua 自动化插件

1. `Aegisub` 自带模板 `Apply karaoke template` (自动化卡拉ok模板)

2. Aegisub自动设置卡拉OK双行字幕样式插件 ([aegisub-set-karaoke-style](https://github.com/MichiyamaKaren/aegisub-set-karaoke-style)) 

> [!TIP]
> 因为原本项目仅支持GUI, 因此我进行了修改, 添加了命令行的调用方式
> 
> 已经放在 [lua/set-karaoke-style.lua](./lua/set-karaoke-style.lua), 应该把他安装到Aegisub安装目录下的`automation/autoload`目录中;

### 1.5 安装歌词爬虫

- [LDDC](https://github.com/chenmozhijin/LDDC) `V0.9.2`

> [!TIP]
> 无需安装, 已经附带于源码

## 二、使用方式

## 三、存在问题
### 3.1 日语不准确

```py
import pykakasi
print(pykakasi.kakasi().convert("あの日"))
```

> 不过该库本意是词法分析... あの日 => あの + 日, 也可以理解. 是 方位词 + 名词 ...
>
> 但是这个狗蛋 `日`, 此时应该读作 `ひ` 而不是 `にち`

解决方案: [`JpMark._fixReadings`](src/mark/jpMark.py)

- 判断: [あの, 日] 选项; 存在就替换掉. 日后都是这样吧. 特化...
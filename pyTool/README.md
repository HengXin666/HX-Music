# Ass 注音工具
## 一、安装依赖

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

## 二、使用说明

关于英语单词映射, 优先使用词典; 您可以手动安装:

```bash
# 在 libs 文件夹
# 克隆仓库 (已经自带)
git clone https://github.com/Patchethium/e2k.git
cd e2k/vendor

# 下载数据
curl -O https://kaikki.org/dictionary/downloads/ja/ja-extract.jsonl.gz
curl -O http://ftp.edrdg.org/pub/Nihongo/edict2.gz

# 解压
gzip -d ja-extract.jsonl.gz
gzip -d edict2.gz

# 生成 katakana_dict.jsonl
python extract.py
```

词典可能有多个对照, 本程序默认取第一个作为片假名.

如果字典没有, 则通过语素转换到片假名.

如果您认为转换效果不好, 可以自行在 [`katakana_dict.jsonl`](./libs/e2k/vendor/katakana_dict.jsonl) 中添加自己的映射.
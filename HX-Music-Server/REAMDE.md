# HX-Music 服务端

## 一、架构
### 1.1 鉴权设计

需要支持登陆注册.

### 1.2 存储架构

> 支持上传和下载歌曲到总库.
>
> 以及歌单
>
> 并且可自动扫描并且生成 Ass 字幕.

1. 歌曲

存储在 `file/music/${物理分类}/${歌名}`, 注意, 物理分类有且仅有一层文件夹

用户可以使用自定义的迁移. 比如直接上传文件夹. 这样会上传到 `${文件夹}/${歌名}`

如果用户是直接拖拽歌曲, 就上传到 `myMusic` 文件夹

2. 歌曲信息

存储在 `file/music/${物理分类}/musicInfo/${歌名}.json`

即 该歌曲路径下的 `musicInfo` 文件夹

3. 歌词信息

存储在 `file/music/${物理分类}/lyrics/${歌名}.ass`

即 该歌曲路径下的 `lyrics` 文件夹

4. 歌单

存储在 `file/musicList/${歌单名称}.json

内部直接存储歌曲引用即可.

### 1.3 数据设计

1. 歌曲索引

需求:

- 支持搜索功能, 用户可以根据 `歌名、歌手、专辑` 搜索歌曲

- 查重, 如果上传了完全相同的歌曲 (?) / 或者名称相同什么的, 支持查重!

我只需要维护 红黑树

```cpp
struct MusicIndexData {
    std::string url;
    std::string musicName;
    std::vector<std::string> singers;
    std::string album;
};

std::list<MusicIndexData> 全部数据;

std::map<std::string, decltype(std::list<MusicIndexData>)::iterator> 歌名;
// ...
```

即可. 获取到 url, 再加载该歌曲的配置文件 等, 获取详细信息~

问题:

这种并不支持模糊搜索

> 查阅网络, 要么写dp 编辑距离. 但是显然时间复杂度太高了; 并且支持 utf-8 也比较难.

降低要求吧...

- [ ] 精确匹配
- [ ] 前缀匹配
- [ ] 子字符串匹配
- [ ] 编辑距离的模糊匹配
- [ ] 正则表达式支持

## 二、接口设计
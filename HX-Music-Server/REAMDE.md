# HX-Music 服务端

## 一、架构
### 1.1 鉴权设计

需要支持登陆注册. (?)

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

歌曲索引信息存储在 `file/db/music.db`

2. 歌曲信息

存储在 `file/musicInfo/${歌id}.json`

即 该歌曲路径下的 `musicInfo` 文件夹

3. 歌词信息

存储在 `file/lyrics/${歌id}.ass`

4. 歌单

存储在 `file/db/musicList.db`

内部直接存储歌曲引用即可.

5. 数据库内容

存储在 `file/db/${数据库名称}.db`

### 1.3 数据设计
#### 1.3.1 歌曲数据

需求:

- 支持搜索功能, 用户可以根据 `歌名、歌手、专辑` 搜索歌曲

- 查重, 如果上传了完全相同的歌曲 (?) / 或者名称相同什么的, 支持查重!

我只需要维护 红黑树

```cpp
// 歌曲数据
struct MusicDO {
    db::PrimaryKey<uint64_t> id;        // 歌曲唯一ID
    std::string path;                   // 歌曲存放路径 (相对于 ~/file/music/)
    std::string musicName;              // 歌名
    std::vector<std::string> singers;   // 歌手
    std::string musicAlbum;             // 专辑
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
- [ ] ~~正则表达式支持~~

并且查找集合应该支持实时的 增加 和 减少.

#### 1.3.2 歌单数据

```cpp
struct MusicListDO {
    db::PrimaryKey<uint64_t> id;            // 歌单id (唯一), 定义本地歌单为默认, 为 `localPlaylist`
    std::string description;                // 歌单描述
    std::vector<uint64_t> songList;         // 歌曲列表
};
```

#### 1.3.3 歌词数据

配置存储为 json 即可

## 二、接口设计

模板:
> 接口描述:
- 请求方式: `GET`/...
- 接口URL: `/`
- 参数描述:
- 返回值描述:

### 2.1 音乐相关接口
#### 2.1.1 下载音乐

> 接口描述: 断点续传下载音乐接口
- 请求方式: `GET`/`HEAD`
- 接口URL: `/music/download/{id}`
- 参数描述: `id` 为音乐的唯一id
- 返回值描述: `文件`

#### 2.1.2 上传音乐

> 接口描述:
- 请求方式: `GET`/...
- 接口URL: `/`
- 参数描述:
- 返回值描述:

#### 2.1.3 删除音乐

> 接口描述:
- 请求方式: `GET`/...
- 接口URL: `/`
- 参数描述:
- 返回值描述:

#### 2.1.4 扫描音乐

> 接口描述: 如果用户手动在目录下添加了音乐; 而不是通过程序上传. 那么就需要扫描!
- 请求方式: `GET`
- 接口URL: `/music/runScan`
- 参数描述: 无
- 返回值描述: ok
- `@todo`: 日后换为 WebSocket, 可以支持进度显示...完成通知...

#### 2.1.5 获取音乐信息

> 接口描述:
- 请求方式: `GET`
- 接口URL: `/music/info/{id}`
- 参数描述: `音乐ID`
- 返回值描述: `Json`

#### 2.1.6 修改音乐信息

> 接口描述:
- 请求方式: `GET`/...
- 接口URL: `/`
- 参数描述:
- 返回值描述:

### 2.2 歌单相关接口
#### 2.2.1 创建歌单

> 接口描述:
- 请求方式: `POST`
- 接口URL: `/musicList/make`
- 参数描述: 在 `Body` 中传递 `Json`
- 返回值描述: `歌单ID`

#### 2.2.2 编辑歌单

> 接口描述:
- 请求方式: `POST`
- 接口URL: `/musicList/update`
- 参数描述: 在 `Body` 中传递 `Json`, 需要明确填写 `歌单ID`
- 返回值描述: `ok`

#### 2.2.3 删除歌单

> 接口描述:
- 请求方式: `POST`/`DEL`
- 接口URL: `/musicList/del/{id}`
- 参数描述: `id` 为 `歌单ID`
- 返回值描述: `ok`

#### 2.2.4 为歌单添加歌曲

> 接口描述:
- 请求方式: `POST`
- 接口URL: `/musicList/{id}/addMusic/{musicId}`
- 参数描述: `id`-`歌单ID`; `musicId`-`歌曲ID`
- 返回值描述: `ok`

#### 2.2.5 为歌单删除歌曲

> 接口描述:
- 请求方式: `POST`/`DEL`
- 接口URL: `/musicList/{id}/delMusic/{musicId}`
- 参数描述: `id`-`歌单ID`; `musicId`-`歌曲ID`
- 返回值描述: `ok`

#### 2.2.6 为歌单交换歌曲位置

> 接口描述:
- 请求方式: `POST`
- 接口URL: `/musicList/{id}/swapMusic`
- 参数描述: `Body` 传递 `Json` (@todo)
- 返回值描述: `ok`

### 2.3 歌词相关接口
#### 2.3.1 获取歌曲歌词

> 接口描述:
- 请求方式: `GET`/...
- 接口URL: `/`
- 参数描述:
- 返回值描述:
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

存储在 `file/db/playlist.db`

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
// 歌单数据
struct PlaylistDO {
    db::PrimaryKey<uint64_t> id;            // 歌单id (唯一), 定义本地歌单为默认, 为 `0`
    std::string name;                       // 歌单名称
    std::string description;                // 歌单描述
    std::vector<uint64_t> songIdList;       // 歌曲Id列表
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

> [!TIP]
> 特别的, 所有 JSON 返回的, 都是:
>
> [`JsonVO<T>`](../include/pojo/vo/JsonVO.hpp#40-45)
> 
> 下面的描述会省略 `JsonVO`, 仅描述 `T` 的类型!

### 2.1 音乐相关接口
#### 2.1.1 下载音乐

> 接口描述: 断点续传下载音乐接口
- 请求方式: `GET`/`HEAD`
- 接口URL: `/music/download/{id}`
- 参数描述: `id` 为音乐的唯一id
- 返回值描述: `文件`

#### 2.1.2 上传音乐

> 接口描述: 初始化音乐任务
- 请求方式: `POST`
- 接口URL: `/music/upload/init`
- 参数描述: 传递 JSON: 包含上传到的文件路径 (相对于`./file/music/`), 以及文件大小.
- 返回值描述: `/music/upload/init` 获取到唯一的任务 ID (`pushId`)

> 接口描述: 上传音乐, 可以获取上传进度, 以及缓存进度.
- 请求方式: (`GET`) => WebSocket
- 接口URL: `/music/upload/push/{pushId}`
- 参数描述: `/music/upload/push/{pushId}` 通过任务id, 上传音乐文件.
- 返回值描述: 进度百分比字符串 (保留5位小数), 当返回 `"1.00000"` (100%) 后, 会返回 id (`Text`)

#### 2.1.3 删除音乐

> 接口描述:
- 请求方式: `GET`/...
- 接口URL: `/`
- 参数描述:
- 返回值描述:

#### 2.1.4 扫描音乐

> 接口描述: 如果用户手动在目录下添加了音乐; 而不是通过程序上传. 那么就需要扫描!
- 请求方式: `ws`
- 接口URL: `/music/runScan/ws`
- 参数描述: 无
- 返回值描述: ok

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

#### 2.1.7 分页查找歌曲

> 接口描述: 分页查找歌曲
- 请求方式: `POST`
- 接口URL: `/music/select`
- 参数描述: Json: {起始Id, 长度}
- 返回值描述: Json

### 2.2 歌单相关接口
#### 2.2.1 创建歌单

> 接口描述: 创建歌单
- 请求方式: `POST`
- 接口URL: `/playlist/make`
- 参数描述: 在 `Body` 中传递 `Json` (`UpdatePlayListVO`)
- 返回值描述: `歌单ID`

#### 2.2.2 编辑歌单

> 接口描述: 编辑歌单
- 请求方式: `POST`
- 接口URL: `/playlist/update`
- 参数描述: 在 `Body` 中传递 `Json`, 需要明确填写 `歌单ID`
- 返回值描述: `ok`

#### 2.2.3 删除歌单

> 接口描述: 删除歌单
- 请求方式: `POST`/`DEL`
- 接口URL: `/playlist/del/{id}`
- 参数描述: `id` 为 `歌单ID`
- 返回值描述: `ok`

#### 2.2.4 获取歌单

> 接口描述: 获取歌单
- 请求方式: `GET`
- 接口URL: `/playlist/select/{id}`
- 参数描述: `歌单ID`
- 返回值描述: 歌单数据`Json`

#### 2.2.5 获取所有歌单

> 接口描述: 获取所有歌单
- 请求方式: `GET`
- 接口URL: `/playlist/selectAll`
- 参数描述: 无
- 返回值描述: 歌单数据`Json`List (PlaylistInfoListVO)

#### 2.2.6 为歌单添加歌曲

> 接口描述: 为歌单添加歌曲
- 请求方式: `POST`
- 接口URL: `/playlist/{id}/addMusic/{musicId}`
- 参数描述: `id`-`歌单ID`; `musicId`-`歌曲ID`
- 返回值描述: `ok`

#### 2.2.7 为歌单删除歌曲

> 接口描述: 为歌单删除歌曲
- 请求方式: `POST`/`DEL`
- 接口URL: `/playlist/{id}/delMusic/{idx}`
- 参数描述: `id`-`歌单ID`; `idx`-`歌曲所在idx`
- 返回值描述: `ok`

#### 2.2.8 完整更新歌单歌曲顺序

> 接口描述: 完整更新歌单歌曲顺序
- 请求方式: `POST`
- 接口URL: `/playlist/updateMusicOrder/{id}`
- 参数描述: `Body` 传递 `Json` songIdList
- 返回值描述: `ok`

### 2.2.9 获取歌单简介

> 接口描述: 获取歌单简介
- 请求方式: `GET`
- 接口URL: `/playlist/info/{id}`
- 参数描述: `id` 歌单id
- 返回值描述: 歌单数据`Json` (PlaylistInfoVO)

### 2.2.10 获取用户创建歌单

- /playlist/selectAll/created

### 2.2.11 获取用户收藏歌单

- /playlist/selectAll/saved

### 2.2.12 完整更新歌单顺序 (用户创建歌单)

> 接口描述: 完整更新歌单歌曲顺序
- 请求方式: `POST`
- 接口URL: `/playlist/updateOrder/created`
- 参数描述: `Body` 传递 `Json` playlistIdList
- 返回值描述: `ok`

### 2.2.13 完整更新歌单顺序 (用户收藏歌单)

> 接口描述: 完整更新歌单歌曲顺序
- 请求方式: `POST`
- 接口URL: `/playlist/updateOrder/saved`
- 参数描述: `Body` 传递 `Json` playlistIdList
- 返回值描述: `ok`

### 2.3 歌词相关接口
#### 2.3.1 获取歌曲歌词

> 接口描述: 获取 Ass 歌词
- 请求方式: `GET`
- 接口URL: `/lyrics/ass/select/{id}`
- 参数描述: `id` 歌曲Id
- 返回值描述: 断点续传 Ass 歌词

#### 2.3.2 网络爬取歌曲歌词并且日语注音卡拉ok化

> 接口描述: 网络爬取歌曲歌词并且日语注音卡拉ok化
- 请求方式: `POST`
- 接口URL: `/lyrics/ass/karaok/{id}`
- 参数描述: `id` 歌曲Id
- 返回值描述: ok (string)

#### 2.3.3 批量爬取所有没有歌词的歌曲的歌词

> 接口描述: 批量爬取所有没有歌词的歌曲的歌词
- 请求方式: `ws`
- 接口URL: `/lyrics/ass/karaok/all/ws`
- 参数描述: 无
- 返回值描述: WebSocket

### 2.4 封面相关接口
#### 2.4.1 获取封面

> 接口描述: 获取封面
- 请求方式: `GET`/`HEAD`
- 接口URL: `/cover/select/{id}`
- 参数描述: `id` 歌曲Id
- 返回值描述: 断点续传的图片

### 2.5 用户登录注册接口
#### 2.5.1 用户注册

> [!TIP]
> 期望使用 NAS 等私有部署情况. 因此注册要求管理员凭证!

> 接口描述: 用户注册
- 请求方式: `POST`
- 接口URL: `/user/add`
- 参数描述: Json: {用户名, 密码, 权限}
- 返回值描述: ok

#### 2.5.2 用户登录

> 接口描述: 用户登录
- 请求方式: `POST`
- 接口URL: `/user/login`
- 参数描述: Json: {用户名, 密码}
- 返回值描述: json str -> token

### 2.5.3 修改密码

> 接口描述: 修改密码, 修改密码后, 之前的凭证全部无效
- 请求方式: `POST`
- 接口URL: `/user/passwdUpdate`
- 参数描述: json: [原密码, 新密码]
- 返回值描述: ok

### 2.5.4 修改用户名

> 接口描述: 修改用户名
- 请求方式: `POST`
- 接口URL: `/user/nameUpdate`
- 参数描述: json: str
- 返回值描述: ok

### 2.5.5 修改签名

> 接口描述: 修改签名
- 请求方式: `POST`
- 接口URL: `/user/signatureUpdate`
- 参数描述: json: str
- 返回值描述: ok

### 2.5.6 删除用户

> 接口描述: 删除用户, 不删除用户创建的歌单.
- 请求方式: `POST`/`DEL`
- 接口URL: `/user/del/{id}`
- 参数描述: 用户id
- 返回值描述: ok

### 2.5.7 获取所有用户列表

> 接口描述: 获取所有用户列表
- 请求方式: `GET`
- 接口URL: `/user/selectAll`
- 参数描述: 用户id
- 返回值描述: ok

### 2.5.8 获取头像

> 接口描述: 获取头像
- 请求方式: `GET`
- 接口URL: `/user/avatar/get`
- 参数描述: token -> userId
- 返回值描述: 头像

### 2.5.9 修改头像

> 接口描述: 修改头像
- 请求方式: `POST`
- 接口URL: `/user/avatar/update`
- 参数描述: token -> userId, `.jpg 头像`
- 返回值描述: ok
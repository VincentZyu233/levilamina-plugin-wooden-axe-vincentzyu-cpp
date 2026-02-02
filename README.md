# WoodenAxe C++ Plugin for LeviLamina

一个简易的 LeviLamina 小木斧插件，用于加载和放置 Sponge Schematic (.schem) 文件。

## 功能

- 使用木斧选择位置（左键设置 pos1，右键设置 pos2）
- 读取 .schem 格式的 schematic 文件
- 在指定位置放置蓝图

## 命令

| 命令 | 说明 | 权限等级 |
|------|------|---------|
| `/walist` | 列出可用的 schematic 文件 | OP |
| `/waload <filename>` | 加载一个 schematic 文件 | OP |
| `/wapaste` | 在 pos1 位置放置已加载的蓝图 | OP |
| `/wapos` | 显示当前选区 | OP |
| `/waclear` | 清除选区和已加载的蓝图 | OP |

## 使用方法

1. 手持木斧
2. 左键点击方块设置 pos1
3. 右键点击方块设置 pos2（可选，用于未来的复制功能）
4. 将 .schem 文件放入 `plugins/wooden-axe/schematics/` 目录
5. 使用 `/walist` 查看可用文件
6. 使用 `/waload <filename>` 加载文件
7. 使用 `/wapaste` 在 pos1 位置放置

## 编译

需要：
- xmake
- MSVC 编译器 (C++20)
- LeviLamina SDK

```bash
xmake
xmake install -o output
```

## 注意事项

- 目前仅支持 Sponge Schematic v2 格式 (.schem)
- Java 到 Bedrock 的方块名转换可能不完整
- 大型 schematic 的放置可能需要一些时间

## 待实现功能

- [ ] 复制区域
- [ ] 旋转/镜像
- [ ] 撤销操作
- [ ] 更完整的 Java -> Bedrock 方块映射

## License

MIT License

# 开发文档

## GitHub Actions 自动构建

### 触发方式

#### 1. 自动触发（推荐）

在 commit message 中包含 `build version <版本号>` 即可触发构建：

```bash
# 示例
git commit -m "fix: 修复某个bug build version 0.1.0"
git commit -m "feat: 新增功能 build version 0.2.0-beta.1"
git commit -m "build version 1.0.0"
```

**格式要求：**
- `build version` 后面跟版本号，用空格分隔
- 版本号不能包含空格
- 大小写不敏感

#### 2. 手动触发

1. 进入 GitHub 仓库 → Actions 页面
2. 选择 "Build LeviLamina Plugin" workflow
3. 点击 "Run workflow"
4. 输入版本号（可选）
5. 点击绿色按钮执行

### 构建产物

- **名称格式**: `wooden-axe-<版本号>-windows-x64`
- **内容**: `bin/wooden-axe/` 目录（包含 DLL 和 manifest.json）
- **下载**: Actions → 对应的 workflow run → Artifacts

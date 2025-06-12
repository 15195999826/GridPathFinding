# StaticMesh管理器使用说明

## 概述

StaticMesh管理器是一个编辑器功能，用于扫描和管理指定路径下的所有StaticMesh资源。它通过监听游戏运行状态，当检测到使用`ABuildGridMapGameMode`时，自动扫描StaticMesh并将数据注入到GameMode中。

## 架构设计

### 模块分离
- **GridPathFinding模块**: 包含运行时功能，不依赖编辑器
- **GridPathFindingEditor模块**: 包含编辑器功能，负责资源扫描和监听

### 工作流程
1. Editor模块启动时监听世界开始播放事件
2. 当检测到使用`ABuildGridMapGameMode`时，直接调用填充函数
3. 扫描StaticMesh并通过`SetAvailableStaticMeshes`方法注入到GameMode
4. GameMode可以通过`GetAvailableStaticMeshes`等方法访问数据

## 功能特性

- **自动监听**: Editor模块自动监听游戏运行状态
- **智能注入**: 检测到`ABuildGridMapGameMode`时自动扫描并注入StaticMesh数据
- **路径简化**: 将完整的资源路径转换为简短的名称（如：`Tree_01`）
- **重名处理**: 自动处理重名情况，添加数字后缀
- **无依赖**: 运行时模块不依赖编辑器模块，避免循环引用
- **蓝图支持**: 提供蓝图函数库，方便在编辑器中使用

## 配置方法

1. 打开项目设置 (Project Settings)
2. 找到 `Plugins > Grid Path Finding` 设置
3. 在 `UsingMeshRootPaths` 中添加要扫描的根路径，例如：
   - `Meshes/Environment`
   - `Meshes/Props`
   - `Meshes/Characters`

## 使用方法

### 在ABuildGridMapGameMode中使用

```cpp
// 获取所有可用的StaticMesh映射
const TMap<FName, FSoftObjectPath>& MeshMappings = GetAvailableStaticMeshes();

// 根据简写名称获取StaticMesh路径
FSoftObjectPath MeshPath = GetStaticMeshPath(FName("Tree_01"));

// 手动触发刷新（通常不需要，系统会自动处理）
RefreshStaticMeshCache();
```

### 在蓝图中使用

直接通过`ABuildGridMapGameMode`的蓝图接口使用：

- `Get Available Static Meshes`: 获取完整的StaticMesh映射表
- `Get Static Mesh Path`: 根据简写名称获取StaticMesh路径

### 完全自动化

系统完全自动化，无需手动操作：
- 游戏运行时自动检测GameMode
- 自动扫描配置路径下的StaticMesh
- 自动填充数据到GameMode

## 自动化流程

### 启动时
1. Editor模块启动
2. 监听世界开始播放事件

### 游戏运行时
1. 世界开始播放时触发`OnWorldBeginPlay`
2. 检查GameMode类型
3. 如果是`ABuildGridMapGameMode`，直接调用填充函数扫描并注入StaticMesh数据

### 完全自动化
系统无需手动刷新，一切都是自动的：
- 每次游戏运行时自动扫描最新的StaticMesh资源
- 自动处理资源变化

## 路径格式说明

### 输入路径格式
- 相对路径：`Meshes/Environment` (相对于Content文件夹)
- 绝对路径：`/Game/Meshes/Environment`

### 输出路径格式
- 完整UE路径：`/Game/Meshes/Environment/Tree_01.Tree_01`
- 简写名称：`Tree_01`

## 重名处理

当发现重名的StaticMesh时，系统会自动添加数字后缀：
- 第一个：`Tree_01`
- 第二个：`Tree_01_1`
- 第三个：`Tree_01_2`

## 性能考虑

- 扫描操作只在检测到`ABuildGridMapGameMode`时执行
- 扫描结果会被缓存在Editor模块中
- 运行时模块不执行任何资源扫描操作
- 只在编辑器环境下工作，不影响打包后的性能

## 注意事项

1. 此功能仅在编辑器环境下可用
2. 需要确保`UsingMeshRootPaths`配置正确
3. 路径必须指向包含StaticMesh资源的文件夹
4. 运行时模块完全独立，不依赖编辑器功能
5. 数据注入是自动的，通常不需要手动干预

## 故障排除

### 扫描不到StaticMesh
1. 检查`UsingMeshRootPaths`配置是否正确
2. 确认路径下确实存在StaticMesh资源
3. 检查资源是否已经被UE的资源注册表扫描
4. 查看输出日志中的扫描信息

### GameMode中没有数据
1. 确认使用的是`ABuildGridMapGameMode`
2. 检查Editor模块是否正常启动
3. 查看日志中是否有监听器的相关信息
4. 尝试手动调用`RefreshStaticMeshCache()`

### 路径格式错误
1. 确保路径使用正斜杠 `/`
2. 路径不要以斜杠开头（除非是绝对路径）
3. 检查路径是否存在拼写错误

### 编译错误
1. 确保项目包含了GridPathFindingEditor模块
2. 检查是否在正确的编译环境下（编辑器构建）
3. 确认模块间没有循环依赖

## 扩展说明

如果需要支持其他类型的GameMode，可以：

1. 修改`FBuildGridMapGameModeWatcher::CheckAndUpdateGameMode`方法
2. 添加对其他GameMode类型的检测
3. 为其他GameMode添加类似的`SetAvailableStaticMeshes`方法

这种设计保持了模块间的清晰分离，确保了运行时的独立性。
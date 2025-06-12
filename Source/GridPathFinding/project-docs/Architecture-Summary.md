# StaticMesh管理器极简架构

## 设计原则

### 极简主义
- **无多余抽象**: 只保留最核心的功能
- **直接有效**: 游戏运行时自动填充数据
- **最小组件**: 只有两个核心组件

## 核心组件

1. **FGridPathFindingEditorModule** (Editor模块)
   - 监听世界开始播放事件
   - 检测ABuildGridMapGameMode
   - 直接扫描StaticMesh资源
   - 填充数据到GameMode

2. **ABuildGridMapGameMode** (运行时模块)
   - 存储StaticMesh映射数据
   - 提供访问接口

## 极简工作流程

```
Editor模块启动 → 监听世界事件 → 检测GameMode → 直接填充数据
```

## 优势

- **极简设计**: 只有两个核心组件
- **自动化**: 游戏运行时自动处理，无需手动干预
- **零配置**: 配置好路径后完全自动化
- **高效直接**: 没有任何多余的抽象层

## 核心代码

### Editor模块监听
```cpp
void FGridPathFindingEditorModule::StartupModule()
{
    // 监听世界初始化事件
    WorldInitializedHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FGridPathFindingEditorModule::OnWorldInitialized);
}

void FGridPathFindingEditorModule::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
    // 检查是否是BuildGridMapGameMode
    if (ABuildGridMapGameMode* GameMode = Cast<ABuildGridMapGameMode>(World->GetAuthGameMode()))
    {
        FillStaticMeshData(GameMode);
    }
}
```

### 直接填充数据
```cpp
void FGridPathFindingEditorModule::FillStaticMeshData(ABuildGridMapGameMode* GameMode)
{
    // 直接扫描StaticMesh
    TMap<FName, FSoftObjectPath> StaticMeshes = ScanStaticMeshes(Settings->UsingMeshRootPaths);
    
    // 填充到GameMode
    GameMode->SetAvailableStaticMeshes(StaticMeshes);
}
```

这种设计更加简洁，符合"简单就是美"的原则。
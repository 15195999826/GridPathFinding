#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/World.h"

class ABuildGridMapGameMode;
class UWorld;

class FGridPathFindingEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    // 世界Actor初始化完成时的回调
    void OnWorldInitializedActors(const FActorsInitializedParams& Params);
    
    // 填充StaticMesh数据到GameMode
    void FillStaticMeshData(ABuildGridMapGameMode* GameMode);
    
    // 扫描StaticMesh资源
    TMap<FName, FSoftObjectPath> ScanStaticMeshes(const TArray<FString>& RootPaths);
    
    // 从完整路径生成简写名称
    FName GenerateShortName(const FString& FullPath);
    
    // 委托句柄
    FDelegateHandle WorldInitializedActorsHandle;
};

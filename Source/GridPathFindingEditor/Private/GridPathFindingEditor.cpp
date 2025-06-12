#include "GridPathFindingEditor.h"
#include "GridPathFindingSettings.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FGridPathFindingEditorModule"

void FGridPathFindingEditorModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::StartupModule] Starting GameMode monitoring"));
    
    // 监听世界Actor初始化完成事件（此时GameMode已经创建并且所有Actor都已初始化）
    WorldInitializedActorsHandle = FWorldDelegates::OnWorldInitializedActors.AddRaw(this, &FGridPathFindingEditorModule::OnWorldInitializedActors);
}

void FGridPathFindingEditorModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::ShutdownModule] Stopping GameMode monitoring"));
    
    // 移除监听
    if (WorldInitializedActorsHandle.IsValid())
    {
        FWorldDelegates::OnWorldInitializedActors.Remove(WorldInitializedActorsHandle);
    }
}

void FGridPathFindingEditorModule::OnWorldInitializedActors(const FActorsInitializedParams& Params)
{
    UWorld* World = Params.World;
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] Called with World: %s"), World ? *World->GetName() : TEXT("NULL"));
    
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] World is NULL, returning"));
        return;
    }
    
    if (World->GetNetMode() == NM_DedicatedServer)
    {
        UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] Dedicated server mode, returning"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] World NetMode: %d"), (int32)World->GetNetMode());
    
    // 检查是否是BuildGridMapGameMode
    auto CurrentGameMode = World->GetAuthGameMode();
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] Current GameMode: %s"), 
        CurrentGameMode ? *CurrentGameMode->GetClass()->GetName() : TEXT("NULL"));
    
    if (ABuildGridMapGameMode* GameMode = Cast<ABuildGridMapGameMode>(CurrentGameMode))
    {
        UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] Found BuildGridMapGameMode, filling StaticMesh data"));
        FillStaticMeshData(GameMode);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::OnWorldInitializedActors] Not BuildGridMapGameMode, skipping"));
    }
}

void FGridPathFindingEditorModule::FillStaticMeshData(ABuildGridMapGameMode* GameMode)
{
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] Called"));
    
    if (!GameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] GameMode is NULL, returning"));
        return;
    }
    
    // 获取设置
    const UGridPathFindingSettings* Settings = GetDefault<UGridPathFindingSettings>();
    
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] UsingMeshRootPaths count: %d"), Settings->UsingMeshRootPaths.Num());
    
    for (int32 i = 0; i < Settings->UsingMeshRootPaths.Num(); i++)
    {
        UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] RootPath[%d]: %s"), i, *Settings->UsingMeshRootPaths[i]);
    }
    
    if (Settings->UsingMeshRootPaths.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] UsingMeshRootPaths is empty, returning"));
        return;
    }
    
    // 直接扫描StaticMesh
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] Starting StaticMesh scan"));
    TMap<FName, FSoftObjectPath> StaticMeshes = ScanStaticMeshes(Settings->UsingMeshRootPaths);
    
    // 填充到GameMode
    GameMode->SetAvailableStaticMeshes(StaticMeshes);
    
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::FillStaticMeshData] Successfully filled %d StaticMesh entries to GameMode"), StaticMeshes.Num());
    
    // 打印所有注册的Mesh信息
    UE_LOG(LogTemp, Log, TEXT("========== All Registered StaticMeshes =========="));
    for (const auto& MeshPair : StaticMeshes)
    {
        UE_LOG(LogTemp, Log, TEXT("Key: %s | Path: %s"), 
            *MeshPair.Key.ToString(), 
            *MeshPair.Value.ToString());
    }
    UE_LOG(LogTemp, Log, TEXT("==============================================="));
}

TMap<FName, FSoftObjectPath> FGridPathFindingEditorModule::ScanStaticMeshes(const TArray<FString>& RootPaths)
{
    TMap<FName, FSoftObjectPath> Result;
    
    // 获取资源注册表模块
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    // 直接在指定路径下搜索StaticMesh资源
    int32 TotalCount = 0;
    
    for (const FString& RootPath : RootPaths)
    {
        // 标准化路径格式
        FString NormalizedRootPath = RootPath;
        if (!NormalizedRootPath.StartsWith(TEXT("/Game/")))
        {
            // 如果不是以/Game/开头，假设是相对于Content的路径
            NormalizedRootPath = TEXT("/Game/") + NormalizedRootPath;
        }
        
        UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::ScanStaticMeshes] Scanning path: %s"), *NormalizedRootPath);
        
        // 创建过滤器，只查找指定路径下的StaticMesh资源
        FARFilter Filter;
        Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
        Filter.bRecursiveClasses = true;
        Filter.bRecursivePaths = true;
        Filter.PackagePaths.Add(FName(*NormalizedRootPath));
        
        // 获取指定路径下的StaticMesh资源
        TArray<FAssetData> AssetDataArray;
        AssetRegistry.GetAssets(Filter, AssetDataArray);
        
        UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::ScanStaticMeshes] Found %d StaticMesh assets in path: %s"), 
            AssetDataArray.Num(), *NormalizedRootPath);
        
        // 处理找到的资源
        for (const FAssetData& AssetData : AssetDataArray)
        {
            FString AssetPath = AssetData.GetSoftObjectPath().ToString();
            FName ShortName = GenerateShortName(AssetPath);
            
            // 检查是否有重名，如果有重名则添加后缀
            FName FinalName = ShortName;
            int32 Suffix = 1;
            while (Result.Contains(FinalName))
            {
                FinalName = FName(*FString::Printf(TEXT("%s_%d"), *ShortName.ToString(), Suffix));
                Suffix++;
            }
            
            Result.Add(FinalName, AssetData.GetSoftObjectPath());
            TotalCount++;
            
            UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::ScanStaticMeshes] Added Mesh - Key: %s | Path: %s"), 
                *FinalName.ToString(), *AssetPath);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[FGridPathFindingEditorModule::ScanStaticMeshes] Total %d StaticMesh assets found in specified root paths"), TotalCount);
    
    return Result;
}

FName FGridPathFindingEditorModule::GenerateShortName(const FString& FullPath)
{
    // 从完整路径中提取文件名
    // 例如: "/Game/Meshes/Environment/Tree_01" -> "Tree_01"
    FString FileName = FPaths::GetBaseFilename(FullPath);
    
    // 移除可能的类型后缀（如果有的话）
    // 例如: "Tree_01.Tree_01" -> "Tree_01"
    int32 DotIndex;
    if (FileName.FindChar('.', DotIndex))
    {
        FileName = FileName.Left(DotIndex);
    }
    
    return FName(*FileName);
}



#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGridPathFindingEditorModule, GridPathFindingEditor)
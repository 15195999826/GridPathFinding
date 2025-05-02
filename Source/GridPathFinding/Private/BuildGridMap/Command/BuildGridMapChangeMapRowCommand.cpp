// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeMapRowCommand.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"

class UGridPathFindingSettings;

void UBuildGridMapChangeMapRowCommand::Initialize(int32 InOldRow, int32 InNewRow)
{
	OldRow = InOldRow;
	NewRow = InNewRow;
}

bool UBuildGridMapChangeMapRowCommand::Execute()
{
	if (!CheckValid(NewRow))
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.Execute] 入参不合法， 新行数: %d"), NewRow);
		return false;
	}

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

	// if (NewRow == OldRow)
	if (NewRow == EditingMapSave->MapConfig.MapSize.X)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.Execute] 新行数和旧行数相同， 无需修改"));
		return false;
	}

	// 执行前保存当前地图状态
	SaveCurrentMapState();

	// 获取当前编辑的地块数据
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();
	// // 获取本地保存的地块数据
	// TMap<FHCubeCoord, FSerializableTile> LocalStorageTiles;
	// auto ChunkPath = FPaths::Combine(FPaths::ProjectContentDir(), GetDefault<UGridPathFindingSettings>()->MapSaveFolder, EditingMapSave->ChunksDir);
	// check(FPaths::DirectoryExists(ChunkPath));
	// TArray<FString> ChunkFiles;
	// IFileManager::Get().FindFiles(ChunkFiles, *ChunkPath, TEXT("*.bin"));
	// for (const auto& ChunkFile : ChunkFiles)
	// {
	// 	FString ChunkFilePath = ChunkPath / ChunkFile;
	// 	FGridMapTilesSave ChunkSave;
	// 	if (UGridPathFindingBlueprintFunctionLib::LoadGridMapTilesFromFile(ChunkFilePath, ChunkSave))
	// 	{
	// 		for (const auto& Tile : ChunkSave.GridTiles)
	// 		{
	// 			LocalStorageTiles.Add(Tile.Coord, Tile);
	// 		}
	// 	}
	// }
	// for (const auto& TilePair : EditingTiles)
	// {
	// 	UE_LOG(LogGridPathFinding, Log, TEXT("Old TileCoord: %s"), *TilePair.Key.ToString());
	// }

	// 更新地图配置中的行数
	EditingMapSave->MapConfig.MapSize.X = NewRow;

	// 根据行数调整地块数据
	TMap<FHCubeCoord, FSerializableTile> UpdatedTiles;
	for (const auto& TilePair : EditingTiles)
	{
		// 筛选新区域内的地块
		if (UGridPathFindingBlueprintFunctionLib::IsCoordInMapArea(EditingMapSave->MapConfig, TilePair.Key))
		{
			// UE_LOG(LogGridPathFinding, Log, TEXT("New TileCoord: %s"), *TilePair.Key.ToString());
			UpdatedTiles.Add(TilePair.Key, TilePair.Value);
		}
	}

	// 更新地块配置
	MyGameMode->SetEditingTiles(MoveTemp(UpdatedTiles));
	

	// 应用并渲染新地图
	MyGameMode->GridMapModel->BuildTilesData(EditingMapSave->MapConfig, MyGameMode->GetEditingTiles());
	MyGameMode->BuildGridMapRenderer->RenderGridMap();

	// 保存Map和Tiles数据
	// MyGameMode->IntervalMapSaveToFile(*EditingMapSave);
	// MyGameMode->SaveEditingMapSave(EBuildGridMapSaveMode::FullSave);

	// 广播到UI
	MyGameMode->GetCommandManager()->OnChangeMapSizeX.Broadcast(OldRow, NewRow);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.Execute] Success"));
	return true;
}

bool UBuildGridMapChangeMapRowCommand::Undo()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.Undo] 正在撤销地图行数修改（从 %d 到 %d）"), NewRow, OldRow);
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

	// 恢复旧的行数
	EditingMapSave->MapConfig.MapSize.X = OldRow;

	// 恢复保存的地图状态
	RestoreMapState();

	// 重新渲染地图
	MyGameMode->BuildGridMapRenderer->RenderGridMap();

	// 保存Map和Tiles数据
	// MyGameMode->IntervalMapSaveToFile(*EditingMapSave);
	// MyGameMode->SaveEditingMapSave(EBuildGridMapSaveMode::FullSave);

	// 广播到UI
	MyGameMode->GetCommandManager()->OnChangeMapSizeX.Broadcast(NewRow, OldRow);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.Undo] 成功撤销地图行数修改"));
	return true;
}

FString UBuildGridMapChangeMapRowCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地图行数（从 %d 到 %d）"), OldRow, NewRow);
}

// 保存当前地图状态
void UBuildGridMapChangeMapRowCommand::SaveCurrentMapState()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();
	// auto ChunkDir = FPaths::Combine(FPaths::ProjectContentDir(), GetDefault<UGridPathFindingSettings>()->MapSaveFolder, EditingMapSave->ChunksDir);
	//
	// if (!FPaths::DirectoryExists(ChunkDir))
	// {
	// 	UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapChangeMapRowCommand.SaveCurrentMapState] 地图块目录不存在: %s"), *ChunkDir);
	// 	return;
	// }

	// 清空之前的备份
	SavedTiles.Empty();

	// 加载所有地图块
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();
	for (const auto& Tile : EditingTiles)
	{
		SavedTiles.Add(Tile.Key, Tile.Value);
	}

	// 保存当前地图配置
	SavedMapSize = EditingMapSave->MapConfig.MapSize;

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.SaveCurrentMapState] 成功保存地图状态，共 %d 个格子"), SavedTiles.Num());
}

// 恢复地图状态
void UBuildGridMapChangeMapRowCommand::RestoreMapState()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

	// 恢复地图配置
	EditingMapSave->MapConfig.MapSize = SavedMapSize;

	// 应用保存的格子数据
	MyGameMode->GridMapModel->BuildTilesData(EditingMapSave->MapConfig, SavedTiles);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapRowCommand.RestoreMapState] 成功恢复地图状态，共 %d 个格子"), SavedTiles.Num());
}

bool UBuildGridMapChangeMapRowCommand::CheckValid(int32 InRow)
{
	constexpr int32 MaxRow = 10000;
	if (InRow > 0 && InRow < MaxRow)
	{
		return true;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapChangeMapRowCommand.CheckValid] invalid InRow: %d"), InRow);
	return false;
}

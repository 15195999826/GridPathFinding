// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeMapColCommand.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"

void UBuildGridMapChangeMapColCommand::Initialize(int32 InOldCol, int32 InNewCol)
{
	OldCol = InOldCol;
	NewCol = InNewCol;
}

bool UBuildGridMapChangeMapColCommand::Execute()
{
	if (!CheckValid(NewCol))
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.Execute] 入参不合法， 新列数: %d"), NewCol);
		return false;
	}

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

	// if (NewCol == OldCol)
	if (NewCol == EditingMapSave->MapConfig.MapSize.Y)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.Execute] 新列数和旧列数相同， 无需修改"));
		return false;
	}

	// 执行前保存当前地图状态
	SaveCurrentMapState();

	// 获取当前编辑的地块数据
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

	// 更新地图配置中的列数
	EditingMapSave->MapConfig.MapSize.Y = NewCol;

	// 根据列数调整地块数据
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

	// 广播到UI
	MyGameMode->GetCommandManager()->OnChangeMapSizeY.Broadcast(OldCol, NewCol);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.Execute] Success"));
	return true;
}

bool UBuildGridMapChangeMapColCommand::Undo()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.Undo] 正在撤销地图列数修改（从 %d 到 %d）"), NewCol, OldCol);
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

	// 恢复旧的列数
	EditingMapSave->MapConfig.MapSize.Y = OldCol;

	// 恢复保存的地图状态
	RestoreMapState();

	// 重新渲染地图
	MyGameMode->BuildGridMapRenderer->RenderGridMap();

	// 广播到UI
	MyGameMode->GetCommandManager()->OnChangeMapSizeY.Broadcast(NewCol, OldCol);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.Undo] 成功撤销地图列数修改"));
	return true;
}

FString UBuildGridMapChangeMapColCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地图列数（从 %d 到 %d）"), OldCol, NewCol);
}

void UBuildGridMapChangeMapColCommand::SaveCurrentMapState()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

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

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.SaveCurrentMapState] 成功保存地图状态，共 %d 个格子"), SavedTiles.Num());
}

void UBuildGridMapChangeMapColCommand::RestoreMapState()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();

	// 恢复地图配置
	EditingMapSave->MapConfig.MapSize = SavedMapSize;

	// 应用保存的格子数据
	MyGameMode->GridMapModel->BuildTilesData(EditingMapSave->MapConfig, SavedTiles);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeMapColCommand.RestoreMapState] 成功恢复地图状态，共 %d 个格子"), SavedTiles.Num());
}

bool UBuildGridMapChangeMapColCommand::CheckValid(int32 InCol)
{
	constexpr int32 MaxCol = 10000;
	if (InCol > 0 && InCol < MaxCol)
	{
		return true;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapChangeMapColCommand.CheckValid] invalid InCol: %d"), InCol);
	return false;
}

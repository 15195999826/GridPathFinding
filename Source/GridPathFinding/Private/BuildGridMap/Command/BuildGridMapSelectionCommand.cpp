// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapSelectionCommand.h"

#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapPlayerController.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"

void UBuildGridMapSelectionCommand::Initialize(const TArray<FHCubeCoord>& InNewSelectedTiles)
{
	SelectedTiles = InNewSelectedTiles;

	if (auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PreviousTiles = PC->SelectionComponent->GetSelectedTilesCopy();
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapSelectionCommand.Initialize] PC is invalid!"));
	}
}

bool UBuildGridMapSelectionCommand::Execute()
{
	// 判断合法性
	if (SelectedTiles.Num() <= 0)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapSelectionCommand.Execute] No tiles selected!"));
		return false;
	}

	auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapSelectionCommand.Execute] PC is invalid!"));
		return false;
	}

	// 取消当前选中的高亮
	UnHighLight(PreviousTiles);

	// 高亮当前的选中
	HighLight(SelectedTiles);

	// 展示面板
	ShowTileDetail(SelectedTiles);

	// 更新当前选择
	PC->SelectionComponent->SetSelectedTiles(SelectedTiles);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapSelectionCommand.Execute]"));
	return true;
}

bool UBuildGridMapSelectionCommand::Undo()
{
	auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapSelectionCommand.Undo] PC is invalid!"));
		return false;
	}

	// 取消当前选中的高亮
	UnHighLight(SelectedTiles);

	// 重新高亮之前选中的地块
	HighLight(PreviousTiles);

	// 展示面板
	ShowTileDetail(PreviousTiles);

	// 恢复之前选择
	PC->SelectionComponent->SetSelectedTiles(PreviousTiles);

	return true;
}

FString UBuildGridMapSelectionCommand::GetDescription() const
{
	return FString::Printf(TEXT("选择了%d个地块"), SelectedTiles.Num());
}

void UBuildGridMapSelectionCommand::UnHighLight(const TArray<FHCubeCoord>& TileCoords)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapSelectionCommand.UnHighLightTiles] Total: %d"), TileCoords.Num());
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	for (const FHCubeCoord& Coord : TileCoords)
	{
		MyGameMode->BuildGridMapRenderer->HighLightBackground(Coord, false);
	}
}

void UBuildGridMapSelectionCommand::HighLight(const TArray<FHCubeCoord>& TileCoords)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapSelectionCommand.HighLightTiles] Total: %d"), TileCoords.Num());
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	for (const FHCubeCoord& Coord : TileCoords)
	{
		MyGameMode->BuildGridMapRenderer->HighLightBackground(Coord, true);
	}
}

// 判断所有 FHCubeCoord 元素是否相同
bool UBuildGridMapSelectionCommand::IsAllCoordsEqual(const TArray<FSerializableTile>& TileDetails) const
{
	bool bResult = true;
	if (TileDetails.Num() <= 1)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapSelectionCommand.IsAllCoordsEqual] result: %d"), bResult);
		return bResult;
	}

	auto FirstTile = TileDetails[0];
	for (int32 i = 1; i < TileDetails.Num(); ++i)
	{
		if (FirstTile != TileDetails[i])
		{
			bResult = false;
			break;
		}
	}

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapSelectionCommand.IsAllCoordsEqual] result: %d"), bResult);
	return bResult;
}

// 展示地块详情
void UBuildGridMapSelectionCommand::ShowTileDetail(const TArray<FHCubeCoord>& TileCoords)
{
	if (TileCoords.IsEmpty())
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapSelectionCommand.ShowTileDetail] TileCoords Invalid"));
		return;
	}

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapSelectionCommand.ShowTileDetail] TileCoords.Num: %d"), TileCoords.Num());
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();

	// 一个格子的信息
	if (TileCoords.Num() <= 1)
	{
		MyGameMode->GetBuildGridMapWindow()->SingleSelectTileCoord(TileCoords[0]);
		return;
	}

	// 多个格子的信息
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();
	// 判断入参中是否选择了编辑中的格子
	TArray<FSerializableTile> SelectedTilesInEditing;
	for (const FHCubeCoord& Coord : TileCoords)
	{
		const FSerializableTile* Tile = EditingTiles.Find(Coord);
		if (Tile)
		{
			SelectedTilesInEditing.Add(*Tile);
		}
	}

	if (SelectedTilesInEditing.Num() <= 0 || !IsAllCoordsEqual(SelectedTilesInEditing))
	{
		// 没选中格子或者选中的格子数据不相同，展示空格子数据
		FSerializableTile MockTile;
		FTileEnvData MockEnvData;
		MockEnvData.TextureIndex = -1;
		MockTile.TileEnvData = MoveTemp(MockEnvData);
		MyGameMode->GetBuildGridMapWindow()->SingleSelectTileData(MockTile);
	}
	else
	{
		// 选中了有数据的格子且数据都相同，展示其中一个格子的信息
		MyGameMode->GetBuildGridMapWindow()->SingleSelectTileData(SelectedTilesInEditing[0]);
	}
}

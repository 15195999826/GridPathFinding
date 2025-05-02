// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeMultiTileEnvCommand.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"

void UBuildGridMapChangeMultiTileEnvCommand::Initialize(TArray<FHCubeCoord>&& InSelectedCoords, const FName& InNewTileEnv)
{
	SelectedCoords = MoveTemp(InSelectedCoords);
	NewTileEnv = InNewTileEnv;

	if (SelectedCoords.Num() > 0)
	{
		auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
		const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

		// 保存原始的地块环境
		OldTileEnvMap.Reserve(SelectedCoords.Num());
		for (int32 i = 0; i < SelectedCoords.Num(); ++i)
		{
			if (EditingTiles.Contains(SelectedCoords[i]))
			{
				OldTileEnvMap.Add(SelectedCoords[i], EditingTiles[SelectedCoords[i]].EnvironmentType);
			}
			else
			{
				OldTileEnvMap.Add(SelectedCoords[i], UGridEnvironmentType::EmptyEnvTypeID);
			}
		}
	}
}

bool UBuildGridMapChangeMultiTileEnvCommand::Execute()
{
	if (SelectedCoords.IsEmpty())
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeEnvCommand.Execute] Selected Coords is empty!"));
		return false;
	}

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

	// 移除地块操作
	if (NewTileEnv == UGridEnvironmentType::EmptyEnvTypeID)
	{
		// 批量执行
		for (int32 i = 0; i < SelectedCoords.Num(); ++i)
		{
			const FHCubeCoord& Coord = SelectedCoords[i];
			if (EditingTiles.Contains(Coord))
			{
				auto OldTile = EditingTiles[Coord];
				MyGameMode->GetMutEditingTiles().Remove(Coord);

				MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->BindSingleTileCoord(Coord);
				MyGameMode->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Remove, OldTile);
				MyGameMode->MarkEditingTilesDirty(Coord);

				UE_LOG(LogGridPathFinding, Log, TEXT("[Execute] Remove"));
			}
		}
	}
	else // 更新地块操作
	{
		// 批量执行
		for (int32 i = 0; i < SelectedCoords.Num(); ++i)
		{
			const FHCubeCoord& Coord = SelectedCoords[i];
			// 修改
			if (EditingTiles.Contains(Coord))
			{
				MyGameMode->GetMutEditingTiles()[Coord].EnvironmentType = NewTileEnv;

				MyGameMode->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Update, EditingTiles[Coord]);
				UE_LOG(LogGridPathFinding, Log, TEXT("[Execute] Update"));
			}
			else // 新增
			{
				auto NewTile = FSerializableTile();
				NewTile.Coord = Coord;
				NewTile.EnvironmentType = NewTileEnv;
				MyGameMode->GetMutEditingTiles().Add(Coord, NewTile);

				MyGameMode->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Add, NewTile);
				UE_LOG(LogGridPathFinding, Log, TEXT("[Execute] Add"));
			}
			MyGameMode->MarkEditingTilesDirty(Coord);
		}
	}

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeEnvCommand.Execute]"));
	return true;
}

bool UBuildGridMapChangeMultiTileEnvCommand::Undo()
{
	if (SelectedCoords.IsEmpty())
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeEnvCommand.Undo] Selected Coords is empty!"));
		return false;
	}

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

	// 批量撤销
	for (const auto& OldTilePair : OldTileEnvMap)
	{
		const FHCubeCoord& Coord = OldTilePair.Key;
		const FName& OldEnvType = OldTilePair.Value;

		// 恢复为空地块
		if (OldEnvType == UGridEnvironmentType::EmptyEnvTypeID)
		{
			if (EditingTiles.Contains(Coord))
			{
				auto OldTile = EditingTiles[Coord];
				MyGameMode->GetMutEditingTiles().Remove(Coord);

				MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->BindSingleTileCoord(Coord);
				MyGameMode->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Remove, OldTile);
				MyGameMode->MarkEditingTilesDirty(Coord);
			}
		}
		else // 恢复或添加地块
		{
			if (EditingTiles.Contains(Coord))
			{
				// 修改回原来的环境类型
				MyGameMode->GetMutEditingTiles()[Coord].EnvironmentType = OldEnvType;
				MyGameMode->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Update, EditingTiles[Coord]);
			}
			else // 需要重新添加
			{
				auto NewTile = FSerializableTile();
				NewTile.Coord = Coord;
				NewTile.EnvironmentType = OldEnvType;
				MyGameMode->GetMutEditingTiles().Add(Coord, NewTile);

				MyGameMode->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Add, NewTile);
			}
			MyGameMode->MarkEditingTilesDirty(Coord);
		}
	}

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeEnvCommand.Undo]"));
	return true;
}


FString UBuildGridMapChangeMultiTileEnvCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地块环境"));
}

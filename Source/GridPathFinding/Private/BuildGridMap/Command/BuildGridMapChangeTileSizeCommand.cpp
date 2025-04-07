// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeTileSizeCommand.h"

#include "GridMapModel.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"

void UBuildGridMapChangeTileSizeCommand::Initialize(const FVector2D& InOldTileSize, const FVector2D& InNewTileSize)
{
	OldTileSize = InOldTileSize;
	NewTileSize = InNewTileSize;
}

bool UBuildGridMapChangeTileSizeCommand::Execute()
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto MuteMapSave = GM->GetMutEditingMapSave();

	switch (MuteMapSave->MapConfig.MapType) {
		case EGridMapType::HEX_STANDARD:
			{
				MuteMapSave->MapConfig.HexGridRadius = NewTileSize.X;
			}
			break;
		case EGridMapType::SQUARE_STANDARD:
			{
				MuteMapSave->MapConfig.SquareSize = NewTileSize.X;
			}
			break;
		case EGridMapType::RECTANGLE_STANDARD:
		case EGridMapType::RECTANGLE_SIX_DIRECTION:
			{
				MuteMapSave->MapConfig.RectSize = NewTileSize;
			}
			break;
	}

	GM->GridMapModel->BuildTilesData(MuteMapSave->MapConfig, GM->GetEditingTiles());
	GM->BuildGridMapRenderer->RenderGridMap();

	return true;
}

bool UBuildGridMapChangeTileSizeCommand::Undo()
{
	return true;
}

FString UBuildGridMapChangeTileSizeCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地图格子大小（从 %s 到 %s）"),
	                       *OldTileSize.ToString(), *NewTileSize.ToString());
}

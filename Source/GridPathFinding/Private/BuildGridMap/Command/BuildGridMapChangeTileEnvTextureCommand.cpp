// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeTileEnvTextureCommand.h"

#include "GridMapModel.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"

void UBuildGridMapChangeTileEnvTextureCommand::Initialize(const FHCubeCoord& InCoord, int32 InOldTileTextureIndex,
                                                          int32 InNewTileTextureIndex)
{
	Coord = InCoord;
	NewTileTextureIndex = InNewTileTextureIndex;
	OldTileTextureIndex = InOldTileTextureIndex;
}

bool UBuildGridMapChangeTileEnvTextureCommand::Execute()
{
	// 程序上已经在前置逻辑中保证了Coord一定存在
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingTile = GM->GetMutEditingTile(Coord);
	check(EditingTile);
	EditingTile->TileEnvData.TextureIndex = NewTileTextureIndex;

	// 更新GridModel
	GM->GridMapModel->ModifyTilesData(EGridMapModelTileModifyType::Update, *EditingTile);
	
	GM->MarkEditingTilesDirty(Coord);
	return true;
}

bool UBuildGridMapChangeTileEnvTextureCommand::Undo()
{
	return true;
}

FString UBuildGridMapChangeTileEnvTextureCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地块贴图 %s（从 %d 到 %d）"),
		*Coord.ToString(),
		OldTileTextureIndex, 
		NewTileTextureIndex);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeHeightCommand.h"

#include "GridMapModel.h"
#include "BuildGridMap/BuildGridMapGameMode.h"

void UBuildGridMapChangeHeightCommand::Initialize(const FHCubeCoord& InCoord, float InNewHeight)
{
	Coord = InCoord;
	NewHeight = InNewHeight;
}

bool UBuildGridMapChangeHeightCommand::Execute()
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	// 获取当前高度
	auto MuteTile = GM->GetMutEditingTile(Coord);
	OldHeight = MuteTile->Height;

	// 设置新的高度
	MuteTile->Height = NewHeight;

	// 更新GridModel
	GM->GridMapModel->UpdateTileHeight(Coord, NewHeight);
	
	GM->MarkEditingTilesDirty(Coord);
	return true;
}

bool UBuildGridMapChangeHeightCommand::Undo()
{
	return true;
}

FString UBuildGridMapChangeHeightCommand::GetDescription() const
{
	return FString::Printf(TEXT("Change Height of Tile at %s from %.2f to %.2f"), *Coord.ToString(), OldHeight, NewHeight);
}

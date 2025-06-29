// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapAddTokenCommand.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"

void UBuildGridMapAddTokenCommand::Initialize(const FHCubeCoord& InCoord)
{
	SelectedCoord = InCoord;
}

bool UBuildGridMapAddTokenCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	if (!EditingTiles.Contains(SelectedCoord))
	{
		FSerializableTile NewTile;
		NewTile.Coord = SelectedCoord;
		MyGameMode->GetMutEditingTiles().Add(SelectedCoord, NewTile);
		HasCoordFlag = false;
	}

	// 创建一个新的SerializableToken
	FSerializableTokenData NewTokenData;
	EditingTiles[SelectedCoord].SerializableTokens.Add(NewTokenData);
	auto NewTokenDataIndex = EditingTiles[SelectedCoord].SerializableTokens.Num() - 1;
	// 创建一个新的TokenActorPanel
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalCreateTokenActorPanel(NewTokenDataIndex, NewTokenData);
	return true;
}

bool UBuildGridMapAddTokenCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	if (!HasCoordFlag)
	{
		MyGameMode->GetMutEditingTiles().Remove(SelectedCoord);
	}
	// 删除新的SerializableToken
	FSerializableTokenData NewTokenData;
	int32 DeleteTokenDataIndex = EditingTiles[SelectedCoord].SerializableTokens.Num() - 1;
	EditingTiles[SelectedCoord].SerializableTokens.RemoveAt(DeleteTokenDataIndex);
	
	// 删除新的TokenActorPanel
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalDeleteTokenActorPanel(DeleteTokenDataIndex);
	return true;
}

FString UBuildGridMapAddTokenCommand::GetDescription() const
{
	return FString::Printf(TEXT("给地块 %s 增加了一个新的Token"),
	*SelectedCoord.ToString());
}

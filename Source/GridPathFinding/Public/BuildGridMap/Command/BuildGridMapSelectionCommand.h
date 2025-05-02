// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "Types/SerializableTile.h"
#include "BuildGridMapSelectionCommand.generated.h"

/**
 * 选择地块命令
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapSelectionCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const TArray<FHCubeCoord>& InNewSelectedTiles);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;

private:
	TArray<FHCubeCoord> PreviousTiles;
	TArray<FHCubeCoord> SelectedTiles;

	void UnHighLight(const TArray<FHCubeCoord>& TileCoords);
	void HighLight(const TArray<FHCubeCoord>& TileCoords);

	// 判断所有 FHCubeCoord 元素是否相同
	bool IsAllCoordsEqual(const TArray<FSerializableTile>& TileDetails) const;

	// 展示地块详情
	void ShowTileDetail(const TArray<FHCubeCoord>& TileCoords);
};

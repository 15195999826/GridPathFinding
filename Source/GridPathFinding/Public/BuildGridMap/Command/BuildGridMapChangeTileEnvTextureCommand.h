// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "UObject/Object.h"
#include "BuildGridMapChangeTileEnvTextureCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeTileEnvTextureCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	/**
	 * 初始化命令
	 * @param InCoord 格子坐标
	 * @param InOldTileTextureIndex 
	 * @param InNewTileTextureIndex
	 */
	void Initialize(const FHCubeCoord& InCoord, int32 InOldTileTextureIndex,
	                int32 InNewTileTextureIndex);

	//~ Begin IGridMapCommand Interface
	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	//~ End IGridMapCommand Interface

private:
	UPROPERTY()
	FHCubeCoord Coord;
	
	/** 新的地图类型 */
	UPROPERTY()
	int32 NewTileTextureIndex;

	/** 旧的地图类型 */
	UPROPERTY()
	int32 OldTileTextureIndex;
};

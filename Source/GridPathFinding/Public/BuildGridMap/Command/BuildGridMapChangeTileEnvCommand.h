// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "UObject/Object.h"
#include "BuildGridMapChangeTileEnvCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeTileEnvCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	/**
	 * 初始化命令
	 * @param InCoord 格子坐标
	 * @param InNeedNewTile
	 * @param InOldTileEnv 旧的地图名字
	 * @param InNewTileEnv 新的地图名字
	 */
	void Initialize(const FHCubeCoord& InCoord, bool InNeedNewTile,const FName& InOldTileEnv, const FName& InNewTileEnv);

	//~ Begin IGridMapCommand Interface
	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	//~ End IGridMapCommand Interface

private:
	UPROPERTY()
	bool NeedNewTile{false};
	
	UPROPERTY()
	FHCubeCoord Coord;
	
	/** 新的地图类型 */
	UPROPERTY()
	FName NewTileEnv;

	/** 旧的地图类型 */
	UPROPERTY()
	FName OldTileEnv;
};

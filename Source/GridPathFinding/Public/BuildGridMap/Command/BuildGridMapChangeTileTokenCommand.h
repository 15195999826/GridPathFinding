// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/HCubeCoord.h"
#include "IGridMapCommand.h"
#include "Types/SerializableTokenData.h"
#include "BuildGridMapChangeTileTokenCommand.generated.h"

struct FSerializableTile;
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeTileTokenCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const FHCubeCoord& InCoord,const int InActorIndex,const FString& InNewTileToken);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;

private:
	// 选中地块的坐标
	FHCubeCoord SelectedCoord;

	// 新的Token索引位置
	int NewActorIndex;

	//新的Token名称
	FString NewTileTokenName;
	
	// 原始的Token数据
	FSerializableTokenData OldTokenData;
};

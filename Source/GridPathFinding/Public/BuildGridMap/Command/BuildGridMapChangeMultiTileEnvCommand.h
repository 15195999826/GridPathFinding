// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "BuildGridMapChangeMultiTileEnvCommand.generated.h"

/**
 * 批量修改地块环境命令
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeMultiTileEnvCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(TArray<FHCubeCoord>&& InSelectedCoords, const FName& InNewTileEnv);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;

private:
	// 选中地块的坐标列表
	TArray<FHCubeCoord> SelectedCoords;

	// 新的环境类型
	FName NewTileEnv;

	// 原始的地块环境
	TMap<FHCubeCoord, FName> OldTileEnvMap;
};

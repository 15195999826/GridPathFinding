// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/SerializableTile.h"
#include "BuildGridMapChangeMapColCommand.generated.h"

/**
 * 修改地图列数命令
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeMapColCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(int32 InOldCol, int32 InNewCol);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;

private:
	int32 OldCol;
	int32 NewCol;

	// 用于撤销和重做的状态保存
	FIntPoint SavedMapSize;
	TMap<FHCubeCoord, FSerializableTile> SavedTiles;

	// 保存和恢复地图状态的辅助方法
	void SaveCurrentMapState();
	void RestoreMapState();

	// 检查入参合法性
	bool CheckValid(int32 InCol);
};

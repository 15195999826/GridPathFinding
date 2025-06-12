// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "Types/SerializableTile.h"
#include "UObject/Object.h"
#include "BuildGridMapCopyPasteCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapCopyPasteCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()
public:
	/**
	 * 初始化复制粘贴命令
	 * @param InSourceCoord 源地块坐标（被复制的地块）
	 * @param InTargetCoords 目标地块坐标列表（要粘贴到的地块）
	 * @param InSourceTileData 源地块的数据
	 */
	void Initialize(const FHCubeCoord& InSourceCoord, TArray<FHCubeCoord>&& InTargetCoords, const FSerializableTile& InSourceTileData);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;

private:
	// 源地块坐标
	FHCubeCoord SourceCoord;
	
	// 目标地块坐标列表
	TArray<FHCubeCoord> TargetCoords;
	
	// 源地块数据（要复制的数据）
	FSerializableTile SourceTileData;
	
	// 原始目标地块数据（用于撤销操作）
	TMap<FHCubeCoord, FSerializableTile> OriginalTargetTileData;
	
	// 记录哪些目标地块原本不存在（用于撤销时删除）
	TSet<FHCubeCoord> NewlyCreatedTiles;
};

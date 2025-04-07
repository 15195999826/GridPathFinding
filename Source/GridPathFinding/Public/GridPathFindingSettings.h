// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GridPathFindingSettings.generated.h"

class AGridMapRenderer;
class UGridMapBuilder;
class IGridMapLoaderInterface;
class UGridEnvironmentType;
/**
 * 
 */
UCLASS(config=Game, DefaultConfig)
class GRIDPATHFINDING_API UGridPathFindingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "环境类型"))
	TArray<TSoftObjectPtr<UGridEnvironmentType>> EnvironmentTypes;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "地图存档文件夹名"))
	FString MapSaveFolder;

	// Todo: 读取Json文件，创建属性编辑窗口
	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "自定义格子数据Json文件"))
	FString CustomGridDataJsonFile;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "地图Chunk尺寸"))
	FIntPoint MapChunkSize = FIntPoint(25, 25);
};

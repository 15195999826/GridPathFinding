// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "GridMapModel.h"
#include "Types/MapConfig.h"
#include "BuildGridMapChangeMapTypeCommand.generated.h"

/**
 * 修改地图类型命令
 */
UCLASS(BlueprintType)
class GRIDPATHFINDING_API UBuildGridMapChangeMapTypeCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()
	
public:
	UBuildGridMapChangeMapTypeCommand();

	/**
	 * 初始化命令
	 * @param InOldMapType 旧的地图类型
	 * @param InNewMapType 新的地图类型
	 */
	void Initialize(EGridMapType InOldMapType, EGridMapType InNewMapType);

	//~ Begin IGridMapCommand Interface
	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	//~ End IGridMapCommand Interface

private:
	/** 新的地图类型 */
	UPROPERTY()
	EGridMapType NewMapType;

	/** 旧的地图类型 */
	UPROPERTY()
	EGridMapType OldMapType;
}; 
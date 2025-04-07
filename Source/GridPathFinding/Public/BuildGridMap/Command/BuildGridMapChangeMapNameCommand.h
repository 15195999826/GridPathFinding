// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "UObject/Object.h"
#include "BuildGridMapChangeMapNameCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeMapNameCommand : public UObject,  public IGridMapCommand
{
	GENERATED_BODY()

public:
	/**
 	* 初始化命令
 	* @param InOldMapName 旧的地图名字
 	* @param InNewMapName 新的地图名字
 	*/
	void Initialize(const FName& InOldMapName, const FName& InNewMapName);

	//~ Begin IGridMapCommand Interface
	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	//~ End IGridMapCommand Interface

private:
	/** 新的地图类型 */
	UPROPERTY()
	FName NewMapName;

	/** 旧的地图类型 */
	UPROPERTY()
	FName OldMapName;
};

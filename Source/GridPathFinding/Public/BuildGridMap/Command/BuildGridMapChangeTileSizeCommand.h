// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "UObject/Object.h"
#include "BuildGridMapChangeTileSizeCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeTileSizeCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()
public:
	/**
	 * 初始化命令
	 * @param InOldTileSize 
	 * @param InNewTileSize
	 */
	void Initialize(const FVector2D& InOldTileSize, const FVector2D& InNewTileSize);

	//~ Begin IGridMapCommand Interface
	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	//~ End IGridMapCommand Interface

private:
	UPROPERTY()
	FVector2D OldTileSize ;
	
	UPROPERTY()
	FVector2D NewTileSize;
};

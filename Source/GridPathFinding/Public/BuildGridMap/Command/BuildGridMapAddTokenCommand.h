// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/HCubeCoord.h"
#include "IGridMapCommand.h"
#include "BuildGridMapAddTokenCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapAddTokenCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const FHCubeCoord& InCoord);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	
private:
	// 选中地块的坐标
	FHCubeCoord SelectedCoord;

	//地图一开始是否存在这个格子
	bool HasCoordFlag = true;
};

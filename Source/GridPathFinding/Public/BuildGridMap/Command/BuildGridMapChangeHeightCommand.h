// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "UObject/Object.h"
#include "BuildGridMapChangeHeightCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapChangeHeightCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()
	
public:
	void Initialize(const FHCubeCoord& InCoord, float InNewHeight);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	
private:
	
	// 选中地块的坐标
	FHCubeCoord Coord;

	float OldHeight;

	float NewHeight;
};

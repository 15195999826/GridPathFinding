// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/HCubeCoord.h"
#include "IGridMapCommand.h"
#include "BuildGridMapAddPropertyArrayCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapAddPropertyArrayCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const FHCubeCoord& InCoord,const int InActorIndex,
	const int InFeatureIndex,const FName& InPropertyArrayName);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	
private:
	
	// 选中地块的坐标
	FHCubeCoord SelectedCoord;

	// 属性数组名称
	FName PropertyArrayName;

	// Feature索引
	int32 FeatureIndex;

	// Actor索引
	int32 ActorIndex;

	// 增加的数组元素索引
	int32 ArrayIndex;
};

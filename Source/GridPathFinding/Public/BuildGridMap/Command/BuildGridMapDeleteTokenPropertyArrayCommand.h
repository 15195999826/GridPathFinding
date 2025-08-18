// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IGridMapCommand.h"
#include "Types/HCubeCoord.h"
#include "UObject/Object.h"
#include "Types/SerializableTokenData.h"

#include "BuildGridMapDeleteTokenPropertyArrayCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapDeleteTokenPropertyArrayCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const FHCubeCoord& InCoord,const int InActorIndex , const int InFeatureIndex,
		const FName& InPropertyArrayName,const int InArrayIndex);
	
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

	// 删除的数组元素索引
	int32 ArrayIndex;

	// 删除的数组元素数据
	TArray<FSerializableTokenProperty> DeletedPropertyArrayData;
	
};

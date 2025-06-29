// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/HCubeCoord.h"
#include "IGridMapCommand.h"
#include "BuildGirdMapChangeTokenFeaturePropertyCommand.generated.h"

/**
 * 
 */

UCLASS()
class GRIDPATHFINDING_API UBuildGirdMapChangeTokenFeaturePropertyCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const FHCubeCoord& InCoord,int InActorIndex, int InFeatureIndex,const FName& InPropertyName,
	const FString& InValue);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	
private:
	// 选中地块的坐标
	FHCubeCoord SelectedCoord;

	//更改的TokenActor索引位置
	int ActorIndex;

	//更改的TokenComponent索引位置
	int FeatureIndex;

	//更改的数据名称
	FName PropertyName;
	
	//新的TokenFeatureProperty数据
	FString NewValue;

	//旧的TokenFeatureProperty数据
	FString OldValue;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/HCubeCoord.h"
#include "IGridMapCommand.h"
#include "Types/SerializableTokenData.h"
#include "BuildGridMapDeleteTokenCommand.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapDeleteTokenCommand : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	void Initialize(const FHCubeCoord& InCoord,int32 InSerializedTokenIndex);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	
private:
	// 选中地块的坐标
	FHCubeCoord SelectedCoord;

	// 要删除的Token索引
	int32 DeleteSerializedTokenIndex;

	// 删除时是否存在Token
	bool HasTokenFlag = false;

	// 删除的Token数据
	FSerializableTokenData OldTokenData;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HGTypes.h"
#include "IGridMapCommand.h"
#include "Types/MapConfig.h"
#include "BuildGridMapChangeOrientation.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GRIDPATHFINDING_API UBuildGridMapChangeOrientation : public UObject, public IGridMapCommand
{
	GENERATED_BODY()

public:
	UBuildGridMapChangeOrientation();

	void Initialize(ETileOrientationFlag InNewOrientationFlag);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual FString GetDescription() const override;
	
private:
	ETileOrientationFlag OldOrientationFlag;
	ETileOrientationFlag NewOrientationFlag;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BuildTokenFeatureInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UBuildTokenFeatureInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 有些Token功能在地图编辑器中需要一些提示性的功能
 */
class GRIDPATHFINDING_API IBuildTokenFeatureInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void InitBuildGridMapFeature() = 0;
};

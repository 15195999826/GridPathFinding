// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MapModelProvider.generated.h"

class UGridMapModel;
// This class does not need to be modified.
UINTERFACE()
class UMapModelProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GRIDPATHFINDING_API IMapModelProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
     * 获取当前使用的GridMapModel实例
     * @return GridMapModel实例，如果不存在则返回nullptr
     */
	virtual UGridMapModel* GetGridMapModel() const = 0;
};

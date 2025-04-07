// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGridMapCommand.generated.h"

/**
 * 地图编辑命令接口
 * 用于实现撤销和重做功能
 */
UINTERFACE(MinimalAPI)
class UGridMapCommand : public UInterface
{
	GENERATED_BODY()
};

class GRIDPATHFINDING_API IGridMapCommand
{
	GENERATED_BODY()

public:
	/**
	 * 执行命令
	 * @return 是否执行成功
	 */
	virtual bool Execute() = 0;

	/**
	 * 撤销命令
	 * @return 是否撤销成功
	 */
	virtual bool Undo() = 0;

	/**
	 * 获取命令描述
	 * @return 命令描述
	 */
	virtual FString GetDescription() const = 0;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridMapModel.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BuildGridMapBlueprintFunctionLib.generated.h"

/**
 * 命令模式使用示例类
 * 提供了在蓝图中使用命令模式的函数
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapBlueprintFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	/**
	 * 示例：撤销上一个命令
	 * @param CommandManager 命令管理器
	 * @return 是否撤销成功
	 */
	UFUNCTION(BlueprintCallable, Category = "GridPathFinding|Command")
	static bool UndoLastCommand(UBuildGridMapCommandManager* CommandManager)
	{
		if (!CommandManager)
		{
			return false;
		}
		
		return CommandManager->UndoCommand();
	}
	
	/**
	 * 示例：重做上一个撤销的命令
	 * @param CommandManager 命令管理器
	 * @return 是否重做成功
	 */
	UFUNCTION(BlueprintCallable, Category = "GridPathFinding|Command")
	static bool RedoLastCommand(UBuildGridMapCommandManager* CommandManager)
	{
		if (!CommandManager)
		{
			return false;
		}
		
		return CommandManager->RedoCommand();
	}
};
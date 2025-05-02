// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IGridMapCommand.h"
#include "BuildGridMapCommandManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FBuildGridMapTwoNameDelegate, const FName&, const FName&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FBuildGridMapTwoSizeXDelegate, int32, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FBuildGridMapTwoSizeYDelegate, int32, int32);

/**
 * 地图编辑命令管理器
 * 用于管理命令的执行、撤销和重做
 * 为了简化设计难度， 当创建地图、切换地图时， 清空全部的操作历史
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapCommandManager : public UObject
{
	GENERATED_BODY()

public:
	UBuildGridMapCommandManager();

	/**
	 * 执行命令
	 * @param Command 要执行的命令
	 * @return 是否执行成功
	 */
	bool ExecuteCommand(TScriptInterface<IGridMapCommand> Command);

	/**
	 * 撤销命令
	 * @return 是否撤销成功
	 */
	bool UndoCommand();

	/**
	 * 重做命令
	 * @return 是否重做成功
	 */
	bool RedoCommand();

	/**
	 * 是否可以撤销
	 * @return 是否可以撤销
	 */
	bool CanUndo() const;

	/**
	 * 是否可以重做
	 * @return 是否可以重做
	 */
	bool CanRedo() const;

	/**
	 * 清空命令历史
	 */
	void ClearCommandHistory();

	/**
	 * 获取可撤销命令数量
	 * @return 可撤销命令数量
	 */
	int32 GetUndoCommandCount() const;

	/**
	 * 获取可重做命令数量
	 * @return 可重做命令数量
	 */
	int32 GetRedoCommandCount() const;

	/**
	 * 设置最大历史记录数量
	 * @param InMaxHistoryCount 最大历史记录数量
	 */
	void SetMaxHistoryCount(int32 InMaxHistoryCount);

	/**
	 * 获取最大历史记录数量
	 * @return 最大历史记录数量
	 */
	int32 GetMaxHistoryCount() const;

	/**
	 * 获取历史命令信息
	 * @return 
	 */
	FString GetHistoryCommandInfo() const;

private:
	/** 已执行的命令历史 */
	UPROPERTY()
	TArray<TScriptInterface<IGridMapCommand>> CommandHistory;

	/** 当前命令索引，指向最后一个执行的命令 */
	int32 CurrentCommandIndex;

	/** 最大历史记录数量，0表示不限制 */
	int32 MaxHistoryCount;

	/**
	 * 确保历史记录数量不超过最大限制
	 */
	void EnforceHistoryLimit();

	// 事件系统， 所有Command需要抛出的事件都在这里声明
public:
	FBuildGridMapTwoNameDelegate OnChangeMapName;
	FBuildGridMapTwoSizeXDelegate OnChangeMapSizeX;
	FBuildGridMapTwoSizeYDelegate OnChangeMapSizeY;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GridPathFindingService.generated.h"

class UGridMapModel;
class IMapModelProvider;
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UGridPathFindingService : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * 获取全局单例实例
	 */
	static UGridPathFindingService* Get();

	/**
	 * 注册GridMapModel提供者（通常由GameMode调用）
	 * @param Provider 实现了IGridMapModelProvider接口的对象
	 */
	void RegisterProvider(TScriptInterface<IMapModelProvider> Provider);

	/**
	 * 取消注册GridMapModel提供者
	 */
	void UnregisterProvider();

	/**
	 * 获取当前的GridMapModel实例
	 * @return GridMapModel实例，如果没有注册Provider则返回nullptr
	 */
	UGridMapModel* GetGridMapModel() const;

private:
	static UGridPathFindingService* Instance;

	// 当前注册的Provider
	TScriptInterface<IMapModelProvider> CurrentProvider;
};

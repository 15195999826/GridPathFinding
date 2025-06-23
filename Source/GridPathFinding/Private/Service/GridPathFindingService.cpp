// Fill out your copyright notice in the Description page of Project Settings.


#include "Service/GridPathFindingService.h"

#include "GridPathFinding.h"
#include "Service/MapModelProvider.h"

UGridPathFindingService* UGridPathFindingService::Instance = nullptr;

UGridPathFindingService* UGridPathFindingService::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UGridPathFindingService>();
		Instance->AddToRoot(); // 防止被垃圾回收
	}
	
	return Instance;
}

void UGridPathFindingService::RegisterProvider(TScriptInterface<IMapModelProvider> Provider)
{
	CurrentProvider = Provider;
}

void UGridPathFindingService::UnregisterProvider()
{
	CurrentProvider = nullptr;
}

UGridMapModel* UGridPathFindingService::GetGridMapModel() const
{
	if (CurrentProvider.GetInterface())
	{
		return CurrentProvider.GetInterface()->GetGridMapModel();
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("CurrentProvider is not valid or does not implement IMapModelProvider interface."));
	return nullptr;
}

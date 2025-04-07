// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildGridMap/Command/BuildGridMapChangeMapTypeCommand.h"

#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"

UBuildGridMapChangeMapTypeCommand::UBuildGridMapChangeMapTypeCommand():
	NewMapType(EGridMapType::HEX_STANDARD)
	, OldMapType(EGridMapType::HEX_STANDARD)
{
}

void UBuildGridMapChangeMapTypeCommand::Initialize(EGridMapType InOldMapType, EGridMapType InNewMapType)
{
	OldMapType = InOldMapType;
	NewMapType = InNewMapType;
}

bool UBuildGridMapChangeMapTypeCommand::Execute()
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto MutMapSave = GM->GetMutEditingMapSave();
	MutMapSave->MapConfig.MapType = NewMapType;

	// 重设数据层数据
	GM->GridMapModel->BuildTilesData(MutMapSave->MapConfig, GM->GetEditingTiles());
	// Renderer重新渲染
	GM->BuildGridMapRenderer->RenderGridMap();
	
	return true;
}

bool UBuildGridMapChangeMapTypeCommand::Undo()
{
	return true;
}

FString UBuildGridMapChangeMapTypeCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地图类型（从 %s 到 %s）"), 
		*UEnum::GetValueAsString(OldMapType), 
		*UEnum::GetValueAsString(NewMapType));
} 
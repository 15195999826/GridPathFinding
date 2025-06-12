// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeOrientation.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"

UBuildGridMapChangeOrientation::UBuildGridMapChangeOrientation()
{
}

void UBuildGridMapChangeOrientation::Initialize(ETileOrientationFlag InNewOrientationFlag)
{
	NewOrientationFlag = InNewOrientationFlag;
}

bool UBuildGridMapChangeOrientation::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetEditingMapSave();

	if (EditingMapSave->MapConfig.TileOrientation == NewOrientationFlag)
	{
		UE_LOG(LogGridPathFinding, Warning, TEXT("[UBuildGridMapChangeOrientation.Execute] New orientation is same as current orientation"));
		return false;
	}

	// 记录旧的方向
	OldOrientationFlag = EditingMapSave->MapConfig.TileOrientation;

	MyGameMode->GetMutEditingMapSave()->MapConfig.TileOrientation = NewOrientationFlag;

	// 应用并渲染新地图
	MyGameMode->GridMapModel->BuildTilesData(EditingMapSave->MapConfig, MyGameMode->GetEditingTiles());
	MyGameMode->BuildGridMapRenderer->RenderGridMap();

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeOrientation.Execute] success"));
	return true;
}

bool UBuildGridMapChangeOrientation::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetEditingMapSave();

	// 恢复旧的方向
	MyGameMode->GetMutEditingMapSave()->MapConfig.TileOrientation = OldOrientationFlag;

	// 应用并渲染新地图
	MyGameMode->GridMapModel->BuildTilesData(EditingMapSave->MapConfig, MyGameMode->GetEditingTiles());
	MyGameMode->BuildGridMapRenderer->RenderGridMap();

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapChangeOrientation.Undo] success"));
	return true;
}

FString UBuildGridMapChangeOrientation::GetDescription() const
{
	return FString::Printf(TEXT("Change Tile Orientation from to"));
}

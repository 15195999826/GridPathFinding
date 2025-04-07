// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/BuildGridMapPlayerController.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "Framework/Application/NavigationConfig.h"

void ABuildGridMapPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FNavigationConfig& NavConfig = *FSlateApplication::Get().GetNavigationConfig();
	NavConfig.bTabNavigation = false;

	UseSpringCamera("Main");
	WeakGM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	WeakGM->OnSaveStart.AddUObject(this, &ABuildGridMapPlayerController::OnSaveStart);
	WeakGM->OnSaveOver.AddUObject(this, &ABuildGridMapPlayerController::OnSaveOver);
}

void ABuildGridMapPlayerController::RemapHitLocation(FVector& HitLocation)
{
	// 重定向命中位置，Snap到格子地图的格子中心
	HitLocation = WeakGM->GridMapModel->StableSnapToGridLocation(HitLocation);
}

void ABuildGridMapPlayerController::CustomTick(float DeltaSeconds, bool OverWidget, bool IsHitGround,
	const FVector& HitGroundLocation, AActor* InHitActor)
{
	if (CanNotInput)
	{
		return;
	}
	
	if (LeftMouseState == ELomoMouseState::Press)
	{
		// 根据当前选择模式， 确定左键点击行为
		switch (MouseMode) {
			case EMouseMode::SingleSelect:
				{
					auto ClickCoord = WeakGM->GridMapModel->StableWorldToCoord(HitGroundLocation);

					// 检查该格子是否在地图范围内
					auto SelectedCoord = GetFirstSelectedCoord();
					if (SelectedCoord != ClickCoord && WeakGM->GridMapModel->IsCoordInMapArea(ClickCoord))
					{
						// 取消选择
						if (WeakGM->GridMapModel->IsCoordInMapArea(SelectedCoord))
						{
							WeakGM->BuildGridMapRenderer->HighLightBackground(SelectedCoord, false);
						}

						auto OldSelectedCoords = SelectedCoords;
						
						// 当前选择的格子
						SelectedCoords.Empty();
						SelectedCoords.Add(ClickCoord);

						// 确认选择
						WeakGM->BuildGridMapRenderer->HighLightBackground(ClickCoord, true);
						// SelectCoord
						WeakGM->GetBuildGridMapWindow()->SingleSelectTile(ClickCoord);
					}
				}
				break;
			case EMouseMode::MultiSelect:
				break;
			case EMouseMode::Paint:
				break;
		}
	}
}

void ABuildGridMapPlayerController::ChangeMouseMode(EMouseMode InMouseMode)
{
	MouseMode = InMouseMode;
	// Todo: 处理一些边界情况， 比如框选过程中切换mode
}

void ABuildGridMapPlayerController::OnSaveStart(EBuildGridMapSaveMode BuildGridMapSaveMode)
{
	if (BuildGridMapSaveMode == EBuildGridMapSaveMode::FullSave)
	{
		SetCanInput(false);
	}
}

void ABuildGridMapPlayerController::OnSaveOver()
{
	SetCanInput(true);
}

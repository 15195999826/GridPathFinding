// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Components/SelectionComponent.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "Blueprint/UserWidget.h"
#include "BuildGridMap/BuildGridMapPlayerController.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"
#include "BuildGridMap/Command/BuildGridMapSelectionCommand.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "BuildGridMap/UI/SelectionBoxWidget.h"

USelectionComponent::USelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsSelecting = false;
	bIsShiftKeyDown = false;
}

void USelectionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USelectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsSelecting)
	{
		DrawSelectionBox();
	}
}

void USelectionComponent::StartSelection(const FVector2D& MousePosition)
{
	bIsSelecting = true;

	SelectionStartPos = MousePosition;
	SelectionCurrentPos = MousePosition;
	UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.StartSelection] StartPos: %s"), *MousePosition.ToString());
}

void USelectionComponent::UpdateSelection(const FVector2D& MousePosition)
{
	// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.UpdateSelection]"));
	if (bIsSelecting)
	{
		SelectionCurrentPos = MousePosition;
	}
}

// 结束框选并选中格子
void USelectionComponent::EndSelection(const FVector& HitGroundLocation)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.EndSelection]"));
	if (bIsSelecting)
	{
		bIsSelecting = false;

		// 移除选择框
		if (SelectionBoxWidget)
		{
			// SelectionBoxWidget->RemoveFromParent();
			// SelectionBoxWidget = nullptr;
			SelectionBoxWidget->SetSelectionBoxVisible(false);
		}

		auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();

		// 单选
		if (SelectionStartPos == SelectionCurrentPos)
		{
			FHCubeCoord OneTile = MyGameMode->GridMapModel->StableWorldToCoord(HitGroundLocation);
			if (MyGameMode->GridMapModel->IsCoordInMapArea(OneTile))
			{
				TArray<FHCubeCoord> NewSelectedTiles;

				// 如果按下了Shift键，则进行添加或移除操作
				if (bIsShiftKeyDown)
				{
					// 复制当前选中的格子
					NewSelectedTiles = SelectedTiles;

					// 检查是否已经选中了这个格子
					const int32 ExistingIndex = NewSelectedTiles.Find(OneTile);
					if (ExistingIndex != INDEX_NONE)
					{
						// 如果已经选中，则移除
						NewSelectedTiles.RemoveAt(ExistingIndex);
						UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.EndSelection] 移除格子: %s"), *OneTile.ToString());
					}
					else
					{
						// 如果未选中，则添加
						NewSelectedTiles.Add(OneTile);
						UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.EndSelection] 添加格子: %s"), *OneTile.ToString());
					}
				}
				else
				{
					// 不按Shift键时，直接替换选中的格子
					NewSelectedTiles.Add(OneTile);

					// 清除之前高亮的格子
					for (const FHCubeCoord& Coord : SelectedTiles)
					{
						MyGameMode->BuildGridMapRenderer->HighLightBackground(Coord, false);
					}
				}

				// 创建并执行命令
				if (NewSelectedTiles.Num() > 0 || bIsShiftKeyDown)
				{
					auto Command = NewObject<UBuildGridMapSelectionCommand>(MyGameMode->CommandManager);
					Command->Initialize(NewSelectedTiles);
					MyGameMode->CommandManager->ExecuteCommand(Command);
				}
			}
		}
		else //多选
		{
			TArray<FHCubeCoord> BoxSelectedTiles = GetGridCellsInSelectionBox();
			if (BoxSelectedTiles.Num() > 0)
			{
				TArray<FHCubeCoord> NewSelectedTiles;

				// 如果按下了Shift键，则进行添加或移除操作
				if (bIsShiftKeyDown)
				{
					// 复制当前选中的格子
					NewSelectedTiles = SelectedTiles;

					// 对于框选中的每个格子
					for (const FHCubeCoord& Coord : BoxSelectedTiles)
					{
						// 检查是否已经选中了这个格子
						int32 ExistingIndex = NewSelectedTiles.Find(Coord);
						if (ExistingIndex != INDEX_NONE)
						{
							// 如果已经选中，则移除
							NewSelectedTiles.RemoveAt(ExistingIndex);
						}
						else
						{
							// 如果未选中，则添加
							NewSelectedTiles.Add(Coord);
						}
					}

					UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.EndSelection] Shift多选后的格子数: %d"), NewSelectedTiles.Num());
				}
				else
				{
					// 不按Shift键时，直接替换选中的格子
					NewSelectedTiles = BoxSelectedTiles;

					// 清除之前高亮的格子
					for (const FHCubeCoord& Coord : SelectedTiles)
					{
						MyGameMode->BuildGridMapRenderer->HighLightBackground(Coord, false);
					}
				}

				// 创建并执行命令
				auto Command = NewObject<UBuildGridMapSelectionCommand>(MyGameMode->CommandManager);
				Command->Initialize(NewSelectedTiles);
				MyGameMode->CommandManager->ExecuteCommand(Command);
			}
		}
	}
}

void USelectionComponent::CancelSelection()
{
	if (bIsSelecting)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent::CancelSelection]"));
		bIsSelecting = false;

		// 移除选择框
		if (SelectionBoxWidget)
		{
			// SelectionBoxWidget->RemoveFromParent();
			// SelectionBoxWidget = nullptr;
			SelectionBoxWidget->SetSelectionBoxVisible(false);
		}
	}
}

// 绘制选择框
void USelectionComponent::DrawSelectionBox()
{
	// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.DrawSelectionBox]"));
	if (auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		// 检查 SelectionBoxWidgetClass 是否有效
		if (!SelectionBoxWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[USelectionComponent.DrawSelectionBox] SelectionBoxWidgetClass 未设置！请在蓝图中设置有效的 Widget 类"));
			return;
		}

		if (!SelectionBoxWidget)
		{
			// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.DrawSelectionBox] Create BoxWidget"));
			SelectionBoxWidget = CreateWidget<USelectionBoxWidget>(PC, SelectionBoxWidgetClass);
			if (SelectionBoxWidget)
			{
				SelectionBoxWidget->AddToViewport(100);
			}
		}

		if (SelectionBoxWidget)
		{
			// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.DrawSelectionBox] StartPos: %s, CurPos: %s"), *SelectionStartPos.ToString(), *SelectionCurrentPos.ToString());

			// 更新选择框的位置和大小
			SelectionBoxWidget->SetSelectionBoxVisible(true);
			SelectionBoxWidget->UpdateSelectionBox(SelectionStartPos, SelectionCurrentPos);
		}
	}
}

TArray<FHCubeCoord> USelectionComponent::GetGridCellsInSelectionBox()
{
	TArray<FHCubeCoord> Result;
	auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[USelectionComponent.GetGridCellsInSelectionBox] PlayerController is null"));
		return Result;
	}

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

	// 计算选择框的屏幕范围
	float Left = FMath::Min(SelectionStartPos.X, SelectionCurrentPos.X);
	float Top = FMath::Min(SelectionStartPos.Y, SelectionCurrentPos.Y);
	float Right = FMath::Max(SelectionStartPos.X, SelectionCurrentPos.X);
	float Bottom = FMath::Max(SelectionStartPos.Y, SelectionCurrentPos.Y);

	// 遍历所有格子 TODO: 优化为按区域遍历
	UGridPathFindingBlueprintFunctionLib::StableForEachMapGrid(EditingMapSave->MapConfig, [&](const FHCubeCoord& Coord, int32 Row, int32 Col)
	{
		// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.GetGridCellsInSelectionBox] Row: %d, Col: %d"), Row, Col);

		FHTileOrientation TileOrientation = MyGameMode->GridMapModel->GetTileOrientation(EditingMapSave->MapConfig.MapType, EditingMapSave->MapConfig.TileOrientation);
		auto TileLocation = UGridPathFindingBlueprintFunctionLib::StableCoordToWorld(EditingMapSave->MapConfig, TileOrientation, Coord);

		FVector2D TileScreenPos;
		if (PC->ProjectWorldLocationToScreen(TileLocation, TileScreenPos))
		{
			// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.GetGridCellsInSelectionBox] ScreenPos: %s"), *TileScreenPos.ToString());

			// 检查每个格子是否在选择框内
			if (TileScreenPos.X >= Left && TileScreenPos.X <= Right && TileScreenPos.Y >= Top && TileScreenPos.Y <= Bottom)
			{
				// UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.GetGridCellsInSelectionBox] Row: %d, Col: %d"), Row, Col);

				// 根据不同的筛选条件进行处理
				bool needFilter = PC->SelectFilterType != ESelectFilterType::None;
				if (!needFilter)
				{
					Result.Add(Coord);
				}
				else
				{
					switch (PC->SelectFilterType)
					{
					case ESelectFilterType::Tile:
						if (!EditingTiles.Find(Coord))
						{
							Result.Add(Coord);
						}
						break;
					case ESelectFilterType::Volume:
						if (EditingTiles.Find(Coord))
						{
							Result.Add(Coord);
						}
						break;
					default:
						UE_LOG(LogGridPathFinding, Error, TEXT("[USelectionComponent.GetGridCellsInSelectionBox] Invalid FilterType"));
						break;
					}
				}
			}
		}
	});

	UE_LOG(LogGridPathFinding, Log, TEXT("[USelectionComponent.GetGridCellsInSelectionBox] SelectedCnt: %d"), Result.Num());
	return Result;
}

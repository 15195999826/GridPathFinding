// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/SelectionBoxWidget.h"

#include "GridPathFinding.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"

USelectionBoxWidget::USelectionBoxWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USelectionBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!SelectionBorder)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[USelectionBoxWidget.NativeConstruct] SelectionBox invalid!"))
		return;
	}

	SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
}

void USelectionBoxWidget::SetSelectionBoxVisible(bool bVisible)
{
	if (!SelectionBorder)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[USelectionBoxWidget.SetSelectionBoxVisible] SelectionBox invalid!"))
		return;
	}

	SelectionBorder->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void USelectionBoxWidget::UpdateSelectionBox_Implementation(const FVector2D& StartPosition, const FVector2D& CurrentPosition)
{
	if (!SelectionBorder)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[USelectionBoxWidget.UpdateSelectionBox_Impl] SelectionBox invalid!"))
		return;
	}

	FGeometry MyGeometry = GetCachedGeometry();
	// 将屏幕坐标转换为本地坐标
	FVector2D StartAbsolutePos, CurrentAbsolutePos;
	USlateBlueprintLibrary::ScreenToWidgetAbsolute(GetWorld(), StartPosition, StartAbsolutePos);
	USlateBlueprintLibrary::ScreenToWidgetAbsolute(GetWorld(), CurrentPosition, CurrentAbsolutePos);

	FVector2D LocalStartPos = MyGeometry.AbsoluteToLocal(StartAbsolutePos);
	FVector2D LocalCurrentPos = MyGeometry.AbsoluteToLocal(CurrentAbsolutePos);

	// 计算选择框的位置和大小
	FVector2D MinPos(FMath::Min(LocalStartPos.X, LocalCurrentPos.X), FMath::Min(LocalStartPos.Y, LocalCurrentPos.Y));
	FVector2D MaxPos(FMath::Max(LocalStartPos.X, LocalCurrentPos.X), FMath::Max(LocalStartPos.Y, LocalCurrentPos.Y));
	FVector2D Size = MaxPos - MinPos;

	// // 设置选择框的位置和大小
	// SelectionBorder->SetRenderTranslation(MinPos);

	// 设置Border的大小
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SelectionBorder->Slot))
	{
		CanvasSlot->SetPosition(MinPos);
		CanvasSlot->SetSize(Size);
		// CanvasSlot->SetPosition(LocalStartPos);
		// CanvasSlot->SetSize(FVector2D(1000.f, 1000.f));
	}

	// 创建新的Brush并设置属性
	FSlateBrush NewBrush;
	NewBrush.DrawAs = ESlateBrushDrawType::Box;
	NewBrush.TintColor = FLinearColor(0.0f, 0.5f, 1.0f, 0.2f); // 半透明蓝色
	SelectionBorder->SetBrush(NewBrush);
}

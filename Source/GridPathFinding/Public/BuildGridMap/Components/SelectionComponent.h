// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SelectionComponent.generated.h"

struct FHCubeCoord;
/**
 * 可以通过点击左上角按钮，切换选择模式；切换选择模式时，取消当前已经选中的格子
 * 实现框选模式功能， 可以通过长按鼠标左键， 拉出一个方框，左键松开后，选中方框内的所有格子；长按过程中， 按下鼠标右键，取消此次框选过程
 * 在此模式下， 可以通过Shift+左键点击格子的方式， 如果被点击格子已经被选择， 则将它改为未选中状态； 如果该点击格子未被选中，则加入到已选择格子数组中
 * 如果同时被选择的格子格子某个属性数据不同， 则不同的那个属性显示为-（如图所示）；修改地块的环境和贴图时， 所有被选中的格子同时应用该数据
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDPATHFINDING_API USelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USelectionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 开始框选
	void StartSelection(const FVector2D& MousePosition);

	// 更新框选
	void UpdateSelection(const FVector2D& MousePosition);

	// 结束框选并选中格子
	void EndSelection(const FVector& HitGroundLocation);

	// 取消框选
	void CancelSelection();

	// 绘制选择框
	void DrawSelectionBox();

	// 获取选择框内的格子
	TArray<FHCubeCoord> GetGridCellsInSelectionBox();

	// 是否正在框选
	bool IsSelecting() const { return bIsSelecting; }

	// 设置Shift键是否按下
	void SetShiftKeyDown(const bool bIsDown)
	{
		bIsShiftKeyDown = bIsDown;
	}

	// 获取当前已选中的格子
	const TArray<FHCubeCoord>& GetSelectedTiles() const
	{
		return SelectedTiles;
	}

	TArray<FHCubeCoord> GetSelectedTilesCopy()
	{
		return SelectedTiles;
	}

	void SetSelectedTiles(const TArray<FHCubeCoord>& InSelectedTiles)
	{
		SelectedTiles = InSelectedTiles;
	}

	void SetSelectedTiles(TArray<FHCubeCoord>&& InSelectedTiles)
	{
		SelectedTiles = MoveTemp(InSelectedTiles);
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// 拷贝时优化
	const int32 ArraySizeThreshold = 1000;

	bool bIsSelecting;

	bool bIsShiftKeyDown;

	// 框选起点
	FVector2D SelectionStartPos;

	// 框选当前点
	FVector2D SelectionCurrentPos;

	// 已选中的格子
	UPROPERTY()
	TArray<FHCubeCoord> SelectedTiles;

	// 选择框Widget
	UPROPERTY()
	class USelectionBoxWidget* SelectionBoxWidget;

	UPROPERTY(EditDefaultsOnly, Category="Selection")
	TSubclassOf<UUserWidget> SelectionBoxWidgetClass;
};

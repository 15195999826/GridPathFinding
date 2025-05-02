// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/BuildGridMapPlayerController.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Framework/Application/NavigationConfig.h"
#include "InputCoreTypes.h"


ABuildGridMapPlayerController::ABuildGridMapPlayerController()
{
	// 创建框选组件
	SelectionComponent = CreateDefaultSubobject<USelectionComponent>(TEXT("SelectionComponent"));
}

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

void ABuildGridMapPlayerController::SetupInputComponent()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[ABuildGridMapPlayerController.SetupInputComponent]"));
	Super::SetupInputComponent();

	// InputComponent->BindKey(EKeys::Z, IE_Pressed, this, &ABuildGridMapPlayerController::OnKeyBoardZPressedHandler).bExecuteWhenPaused = true;

	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &ABuildGridMapPlayerController::OnKeyBoardLeftShiftPressHandler);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &ABuildGridMapPlayerController::OnKeyBoardLeftShiftReleaseHandler);
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

	switch (LeftMouseState)
	{
	case ELomoMouseState::Invalid:
		break;
	case ELomoMouseState::Idle:
		break;
	case ELomoMouseState::Press:
		OnLeftMousePressHandler(DeltaSeconds, OverWidget, IsHitGround, HitGroundLocation, InHitActor);
		break;
	case ELomoMouseState::Pressing:
		OnLeftMousePressingHandler(DeltaSeconds, OverWidget, IsHitGround, HitGroundLocation, InHitActor);
		break;
	case ELomoMouseState::Release:
		OnLeftMouseReleaseHandler(DeltaSeconds, OverWidget, IsHitGround, HitGroundLocation, InHitActor);
		break;
	default:
		break;
	}

	switch (RightMouseState)
	{
	case ELomoMouseState::Invalid:
		break;
	case ELomoMouseState::Idle:
		break;
	case ELomoMouseState::Press:
		break;
	case ELomoMouseState::Pressing:
		break;
	case ELomoMouseState::Release:
		OnRightMouseReleaseHandler(DeltaSeconds, OverWidget, IsHitGround, HitGroundLocation, InHitActor);
		break;
	default:
		break;
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

void ABuildGridMapPlayerController::OnLeftMousePressHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
                                                            const FVector& HitGroundLocation, AActor* InHitActor)
{
	if (!SelectionComponent)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapPlayerController.OnLeftMousePressHandler] Not Implemented"));
		return;
	}

	FVector2D MousePosition;
	// 使用GetMousePosition获取鼠标在屏幕中的位置
	if (GetMousePosition(MousePosition.X, MousePosition.Y))
	{
		// 获取视口大小
		int32 ViewportSizeX, ViewportSizeY;
		GetViewportSize(ViewportSizeX, ViewportSizeY);

		// 确保鼠标位置在视口范围内
		MousePosition.X = FMath::Clamp(MousePosition.X, 0.0f, static_cast<float>(ViewportSizeX));
		MousePosition.Y = FMath::Clamp(MousePosition.Y, 0.0f, static_cast<float>(ViewportSizeY));

		UE_LOG(LogTemp, Warning, TEXT("Viewport Size: X=%d, Y=%d"), ViewportSizeX, ViewportSizeY);
		UE_LOG(LogTemp, Warning, TEXT("Mouse Position: X=%f, Y=%f"), MousePosition.X, MousePosition.Y);

		SelectionComponent->StartSelection(MousePosition);
	}
}

void ABuildGridMapPlayerController::OnLeftMousePressingHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
                                                               const FVector& HitGroundLocation, AActor* InHitActor)
{
	if (!SelectionComponent)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapPlayerController.OnLeftMousePressingHandler] Not Implemented"));
		return;
	}

	FVector2D MousePosition;
	// 使用GetMousePosition获取鼠标在屏幕中的位置
	if (GetMousePosition(MousePosition.X, MousePosition.Y))
	{
		// 获取视口大小
		int32 ViewportSizeX, ViewportSizeY;
		GetViewportSize(ViewportSizeX, ViewportSizeY);

		// 确保鼠标位置在视口范围内
		MousePosition.X = FMath::Clamp(MousePosition.X, 0.0f, static_cast<float>(ViewportSizeX));
		MousePosition.Y = FMath::Clamp(MousePosition.Y, 0.0f, static_cast<float>(ViewportSizeY));

		SelectionComponent->UpdateSelection(MousePosition);
	}
}

void ABuildGridMapPlayerController::OnLeftMouseReleaseHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
                                                              const FVector& HitGroundLocation, AActor* InHitActor)
{
	if (!SelectionComponent)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapPlayerController.OnLeftMouseReleaseHandler] Not Implemented"));
		return;
	}

	if (SelectionComponent->IsSelecting())
	{
		SelectionComponent->EndSelection(HitGroundLocation);
	}
}

void ABuildGridMapPlayerController::OnRightMouseReleaseHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
                                                               const FVector& HitGroundLocation, AActor* InHitActor)
{
	if (!SelectionComponent)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapPlayerController.OnRightMouseReleaseHandler] Not Implemented"));
		return;
	}

	if (SelectionComponent->IsSelecting())
	{
		SelectionComponent->CancelSelection();
	}
}

void ABuildGridMapPlayerController::OnKeyBoardZPressedHandler()
{
	bool bIsCtrlDown = IsInputKeyDown(EKeys::LeftControl);
	bool bIsShiftDown = IsInputKeyDown(EKeys::LeftShift);

	UE_LOG(LogGridPathFinding, Log, TEXT("[ABuildGridMapPlayerController.OnKeyBoardZPressedHandler] Ctrl: %d, Shift: %d"), bIsCtrlDown, bIsShiftDown);
}

void ABuildGridMapPlayerController::OnKeyBoardLeftShiftPressHandler()
{
	if (!SelectionComponent)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapPlayerController.OnKeyBoardLeftShiftPressHandler] Not Implemented"));
		return;
	}

	SelectionComponent->SetShiftKeyDown(true);
}

void ABuildGridMapPlayerController::OnKeyBoardLeftShiftReleaseHandler()
{
	if (!SelectionComponent)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapPlayerController.OnKeyBoardLeftShiftReleaseHandler] Not Implemented"));
		return;
	}

	SelectionComponent->SetShiftKeyDown(false);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapWindow.h"

#include "EngineUtils.h"
#include "GridMapModel.h"
#include "GridMapRenderer.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "JsonObjectConverter.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"
#include "BuildGridMap/UI/BuildGridMapMapConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UBuildGridMapWindow::NativeConstruct()
{
	Super::NativeConstruct();

	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	GM->OnCreateGridMapSave.AddUObject(this, &UBuildGridMapWindow::OnCreateGridMapSave);
	GM->OnEmptyEditingMapSave.AddUObject(this, &UBuildGridMapWindow::OnEmptyEditingMapSave);
	GM->OnSaveStart.AddUObject(this, &UBuildGridMapWindow::OnSaveStart);
	GM->OnSaveOver.AddUObject(this, &UBuildGridMapWindow::OnSaveOver);
	GM->OnSwitchEditingMapSave.AddUObject(this, &UBuildGridMapWindow::OnSwitchEditingMapSave);

	// 指令相关的事件监听
	auto CommandManager = GM->GetCommandManager();
	CommandManager->OnChangeMapName.AddUObject(this, &UBuildGridMapWindow::OnChangeMapName);
	CommandManager->OnChangeMapSizeX.AddUObject(this, &UBuildGridMapWindow::OnChangeMapSizeX);
	CommandManager->OnChangeMapSizeY.AddUObject(this, &UBuildGridMapWindow::OnChangeMapSizeY);
}

void UBuildGridMapWindow::SingleSelectTileCoord_Implementation(const FHCubeCoord& SelectedCoord)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapWindow.SingleSelectTileCoord_Impl]"));
	TileConfigWidget->BindSingleTileCoord(SelectedCoord);
}

void UBuildGridMapWindow::SingleSelectTileData_Implementation(const FSerializableTile& InTileData)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapWindow.SingleSelectTileData_Impl]"));
	TileConfigWidget->BindSingleTileData(InTileData);
}

void UBuildGridMapWindow::OnEmptyEditingMapSave()
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = GM->GetEditingMapSave();
	EditingMapNameText->SetText(FText::FromString(TEXT("None")));
	// 移除当前地图内容
	MapConfigWidget->SetMapConfig(EditingMapSave->MapName, EditingMapSave->MapConfig);
}

void UBuildGridMapWindow::OnSwitchEditingMapSave()
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = GM->GetEditingMapSave();
	// 更新MapConfigWidget数据
	EditingMapNameText->SetText(FText::FromName(EditingMapSave->MapName));
	MapConfigWidget->SetMapConfig(EditingMapSave->MapName, EditingMapSave->MapConfig);
}

void UBuildGridMapWindow::OnChangeMapName(const FName& InOldName, const FName& InNewName)
{
	EditingMapNameText->SetText(FText::FromName(InNewName));
	ChangeMapGroupButtonText(InOldName, InNewName);
}

void UBuildGridMapWindow::OnChangeMapSizeX(int32 InOldSizeX, int32 InNewSizeX)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapWindow.OnChangeMapSizeX] oldX: %d newX: %d"), InOldSizeX, InNewSizeX);
	MapConfigWidget->MapRowTextBox->SetText(FText::AsNumber(InNewSizeX));
}

void UBuildGridMapWindow::OnChangeMapSizeY(int32 InOldSizeY, int32 InNewSizeY)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapWindow.OnChangeMapSizeY] oldY: %d newY: %d"), InOldSizeY, InNewSizeY);
	MapConfigWidget->MapColumnTextBox->SetText(FText::AsNumber(InNewSizeY));
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapWindow.h"

#include "EngineUtils.h"
#include "GridMapModel.h"
#include "GridMapRenderer.h"
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
}

void UBuildGridMapWindow::SingleSelectTile_Implementation(const FHCubeCoord& SelectedCoord)
{
	TileConfigWidget->BindSingleCoord(SelectedCoord);
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

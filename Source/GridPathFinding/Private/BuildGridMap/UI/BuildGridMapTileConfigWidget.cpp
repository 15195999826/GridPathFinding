// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"

#include "GridEnvironmentType.h"
#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "BuildGridMap/UI/BuildGridMapTokenActorPanel.h"
#include "Components/VerticalBox.h"

void UBuildGridMapTileConfigWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnvComboBox->ClearOptions();
	EnvTypeMap.Empty();
	EnvType2DisplayNameMap.Empty();

	EnvComboBox->AddOption(UGridEnvironmentType::EmptyEnvTypeID.ToString());

	auto Settings = GetDefault<UGridPathFindingSettings>();
	for (const auto& SoftEnvType : Settings->EnvironmentTypes)
	{
		auto EnvType = SoftEnvType.LoadSynchronous();
		if (EnvType)
		{
			auto DisplayName = EnvType->DisplayName.ToString();
			EnvComboBox->AddOption(DisplayName);
			EnvTypeMap.Add(DisplayName, EnvType);
			EnvType2DisplayNameMap.Add(EnvType->TypeID, DisplayName);
		}
	}

	EnvComboBox->SetSelectedIndex(0);

	EnvComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridMapTileConfigWidget::OnEnvTypeSelectionChanged);
}

// 通过坐标展示单格地块数据
void UBuildGridMapTileConfigWidget::BindSingleTileCoord(FHCubeCoord SelectedCoord)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapTileConfigWidget.BindSingleTileCoord]"));

	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto RowCol = GM->GridMapModel->StableCoordToRowColumn(SelectedCoord);
	TileRowText->SetText(FText::FromString(FString::Printf(TEXT("Row: %d"), RowCol.X)));
	TileColumnText->SetText(FText::FromString(FString::Printf(TEXT("Col: %d"), RowCol.Y)));
	auto EditingTile = GM->GetEditingTile(SelectedCoord);

	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapTileConfigWidget.BindSingleTileCoord] EnvType:%s"), *EditingTile.TileEnvData.EnvironmentType.ToString());

	if (EnvType2DisplayNameMap.Contains(EditingTile.TileEnvData.EnvironmentType))
	{
		EnvComboBox->SetSelectedOption(EnvType2DisplayNameMap[EditingTile.TileEnvData.EnvironmentType]);
	}
	else
	{
		EnvComboBox->SetSelectedIndex(0);
	}
	EnvTextureIndexTextBox->SetText(FText::FromString(FString::Printf(TEXT("%d"), EditingTile.TileEnvData.TextureIndex)));

	auto CurrentTokenActorPanels = TokenActorPanelRoot->GetAllChildren();
	for (UWidget* Child : CurrentTokenActorPanels)
	{
		auto TokenActorPanel = Cast<UBuildGridMapTokenActorPanel>(Child);
		TokenActorPanel->Clean();
	}

	TokenActorPanelRoot->ClearChildren();
	
	// 创建包含的TokenActor的Panel
	for (int32 i = 0; i < EditingTile.SerializableTokens.Num(); ++i)
	{
		IntervalCreateTokenActorPanel(i, EditingTile.SerializableTokens[i]);
	}
}

// Todo: 删除这个函数， 多个相同功能的入口
void UBuildGridMapTileConfigWidget::BindSingleTileData(const FSerializableTile& InTileData)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapTileConfigWidget.BindSingleTileData]"));

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto RowCol = MyGameMode->GridMapModel->StableCoordToRowColumn(InTileData.Coord);
	TileRowText->SetText(FText::FromString(FString::Printf(TEXT("Row: %d"), RowCol.X)));
	TileColumnText->SetText(FText::FromString(FString::Printf(TEXT("Col: %d"), RowCol.Y)));
	if (EnvType2DisplayNameMap.Contains(InTileData.TileEnvData.EnvironmentType))
	{
		EnvComboBox->SetSelectedOption(EnvType2DisplayNameMap[InTileData.TileEnvData.EnvironmentType]);
	}
	else
	{
		EnvComboBox->SetSelectedIndex(0);
	}
	EnvTextureIndexTextBox->SetText(FText::FromString(FString::Printf(TEXT("%d"), InTileData.TileEnvData.TextureIndex)));
}

void UBuildGridMapTileConfigWidget::OnEnvTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	switch (SelectionType)
	{
	case ESelectInfo::OnMouseClick:
		{
			// SelectedItem To EnvType
			TObjectPtr<UGridEnvironmentType> EnvType = nullptr;
			if (EnvTypeMap.Contains(SelectedItem))
			{
				EnvType = EnvTypeMap[SelectedItem];
			}
			OnTileEnvChanged.Broadcast(EnvType);
		}
		break;
	default:
		break;
	}
}

void UBuildGridMapTileConfigWidget::IntervalCreateTokenActorPanel(int32 InIndex, const FSerializableTokenData& InTokenData)
{
	auto TokenActorPanel = CreateWidget<UBuildGridMapTokenActorPanel>(this, TokenActorPanelClass);
	TokenActorPanel->BindTokenActor(InIndex, InTokenData);
	TokenActorPanelRoot->AddChildToVerticalBox(TokenActorPanel);
}

void UBuildGridMapTileConfigWidget::IntervalUpdateTokenActorPanel(int InIndex,
	const FSerializableTokenData& InTokenData)
{
	// 查找对应Index的 TokenActorPanel
	auto CurrentTokenActorPanels = TokenActorPanelRoot->GetAllChildren();
	if (CurrentTokenActorPanels.IsValidIndex(InIndex))
	{
		auto TokenActorPanel = Cast<UBuildGridMapTokenActorPanel>(CurrentTokenActorPanels[InIndex]);
		if (TokenActorPanel)
		{
			TokenActorPanel->BindTokenActor(InIndex, InTokenData);
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapTileConfigWidget.IntervalUpdateTokenActorPanel] TokenActorPanel not found at index: %d"), InIndex);
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapTileConfigWidget.IntervalUpdateTokenActorPanel] Index out of range: %d"), InIndex);
	}
}

void UBuildGridMapTileConfigWidget::IntervalDeleteTokenActorPanel(int32 InIndex)
{
	auto CurrentTokenActorPanels = TokenActorPanelRoot->GetAllChildren();
	if (CurrentTokenActorPanels.IsValidIndex(InIndex))
	{
		auto TokenActorPanel = Cast<UBuildGridMapTokenActorPanel>(CurrentTokenActorPanels[InIndex]);
		if (TokenActorPanel)
		{
			TokenActorPanel->Clean();
			TokenActorPanelRoot->RemoveChild(TokenActorPanel);
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Warning, TEXT("[UBuildGridMapTileConfigWidget.IntervalDeleteTokenActorPanel] Index out of range: %d"), InIndex);
	}
}

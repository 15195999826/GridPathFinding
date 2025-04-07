// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"

#include "GridEnvironmentType.h"
#include "GridMapModel.h"
#include "GridPathFindingSettings.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

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

void UBuildGridMapTileConfigWidget::BindSingleCoord(FHCubeCoord SelectedCoord)
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto RowCol = GM->GridMapModel->StableCoordToRowColumn(SelectedCoord);
	TileRowText->SetText(FText::FromString(FString::Printf(TEXT("Row: %d"), RowCol.X)));
	TileColumnText->SetText(FText::FromString(FString::Printf(TEXT("Column: %d"), RowCol.Y)));
	auto EditingTile = GM->GetEditingTile(SelectedCoord);
	if (EnvType2DisplayNameMap.Contains(EditingTile.EnvironmentType))
	{
		EnvComboBox->SetSelectedOption(EnvType2DisplayNameMap[EditingTile.EnvironmentType]);
    }
    else
    {
        EnvComboBox->SetSelectedIndex(0);
	}
	EnvTextureIndexTextBox->SetText(FText::FromString(FString::Printf(TEXT("%d"), EditingTile.TileEnvData.TextureIndex)));
}

void UBuildGridMapTileConfigWidget::OnEnvTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	switch (SelectionType) {
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

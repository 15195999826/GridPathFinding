// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapTokenActorPanel.h"

#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/UI/BuildGridMapFeaturePanel.h"
#include "Components/ComboBoxString.h"
#include "Components/VerticalBox.h"

void UBuildGridMapTokenActorPanel::InitTokenActorPanel()
{
	if (IsInitialized)
	{
		return;
	}
	
	TokenActorTypeComboBox->ClearOptions();
	TokenActorTypeComboBox->AddOption(ABuildGridMapGameMode::NoneString);
	auto BuildGridMapGM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	if (BuildGridMapGM)
	{
		const auto& TokenActorMap = BuildGridMapGM->GetTokenActorTypeStringToIndexMap();
		for (const auto& Pair : TokenActorMap)
		{
			TokenActorTypeComboBox->AddOption(Pair.Key);
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridMapTokenActorPanel.NativeConstruct] BuildGridMapGameMode is nullptr"));
	}
	TokenActorTypeComboBox->SetSelectedIndex(0);

	TokenActorTypeComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridMapTokenActorPanel::OnTokenActorTypeSelectionChanged);

	BuildGridMapGM->ListenToTokenActorChange(this);
	IsInitialized = true;
}

void UBuildGridMapTokenActorPanel::NativeConstruct()
{
	Super::NativeConstruct();
	InitTokenActorPanel();
}

void UBuildGridMapTokenActorPanel::BindTokenActor(int32 InTokeActorIndex, const FSerializableTokenData& TokenData)
{
	InitTokenActorPanel();
	TokenActorIndex = InTokeActorIndex;
	if (TokenData.TokenClass == nullptr)
	{
		TokenActorTypeComboBox->SetSelectedOption(ABuildGridMapGameMode::NoneString);
	}
	else
	{
		TokenActorTypeComboBox->SetSelectedOption(TokenData.TokenClass->GetName());
	}

	for (int32 i = 0; i < FeatureRoot->GetChildrenCount(); ++i)
	{
		auto FeaturePanel = Cast<UBuildGridMapFeaturePanel>(FeatureRoot->GetChildAt(i));
		if (FeaturePanel)
		{
			FeaturePanel->Clean();
		}
	}
	FeatureRoot->ClearChildren();
	
	// 创建对应的PropertyLine
	for (int32 i = 0; i < TokenData.Features.Num(); ++i)
	{
		auto FeaturePanel = CreateWidget<UBuildGridMapFeaturePanel>(this, FeaturePanelClass);
		FeaturePanel->BindFeatureData(i, TokenData.Features[i]);

		// 绑定FeaturePropertyChanged事件
		FeaturePanel->OnFeaturePropertyChanged.AddUObject(this, &UBuildGridMapTokenActorPanel::OnFeaturePropertyChange);
		
		FeatureRoot->AddChildToVerticalBox(FeaturePanel);
	}
}

void UBuildGridMapTokenActorPanel::Clean()
{
	// 移除全部监听
	OnTokenActorTypeChanged.Clear();
	OnTokenFeaturePropertyChanged.Clear();
	OnTokenDeleteClicked.Clear();

	for (int32 i = 0; i < FeatureRoot->GetChildrenCount(); ++i)
	{
		auto FeaturePanel = Cast<UBuildGridMapFeaturePanel>(FeatureRoot->GetChildAt(i));
		if (FeaturePanel)
		{
			FeaturePanel->Clean();
		}
	}
}

void UBuildGridMapTokenActorPanel::OnTokenActorTypeSelectionChanged(FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	switch (SelectionType) {
		case ESelectInfo::OnMouseClick:
			OnTokenActorTypeChanged.Broadcast(TokenActorIndex, SelectedItem);
			break;
		default:
			break;
	}
}

void UBuildGridMapTokenActorPanel::OnFeaturePropertyChange(int InFeatureIndex, const FName& PropertyName,
	const FString& PropertyValue)
{
	OnTokenFeaturePropertyChanged.Broadcast(TokenActorIndex, InFeatureIndex, PropertyName, PropertyValue);
}

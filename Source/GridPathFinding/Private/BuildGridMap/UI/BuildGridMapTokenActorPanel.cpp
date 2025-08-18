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
		FeaturePanel->OnAddTokenPropertyArrayDelegate.AddUObject(this,&UBuildGridMapTokenActorPanel::OnAddTokenPropertyArray);
		FeaturePanel->OnDeletePropertyArraySignature.AddDynamic(this,&UBuildGridMapTokenActorPanel::OnDeleteTokenPropertyArray);
		FeaturePanel->OnPropertyArrayValueChangedSignature.AddDynamic(this,&UBuildGridMapTokenActorPanel::OnTokenPropertyArrayValueChanged);
		FeatureRoot->AddChildToVerticalBox(FeaturePanel);
	}
}

void UBuildGridMapTokenActorPanel::UpdateTokenPropertyArray(const int32 InFeatureIndex, const FName& InArrayName,
		 const FSerializableTokenPropertyArray& InPropertyArrayData)
{
	auto CurrentFeaturePanels = FeatureRoot->GetAllChildren();
	if (CurrentFeaturePanels.IsValidIndex(InFeatureIndex))
	{
		auto FeaturePanel = Cast<UBuildGridMapFeaturePanel>(CurrentFeaturePanels[InFeatureIndex]);
		if (FeaturePanel)
		{
			FeaturePanel->UpdateTokenPropertyArray(InArrayName, InPropertyArrayData);
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("[BuildGridMapTokenActorPanel.IntervalAddTokenPropertyArray] FeaturePanel not found at index: %d"), InFeatureIndex);
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[BuildGridMapTokenActorPanel.IntervalAddTokenPropertyArray] Index out of range: %d"), InFeatureIndex);
	}
}

void UBuildGridMapTokenActorPanel::UpdateTokenProperty(const int32 InFeatureIndex,const FSerializableTokenProperty& InPropertyData)
{
	auto CurrentFeaturePanels = FeatureRoot->GetAllChildren();
	if (CurrentFeaturePanels.IsValidIndex(InFeatureIndex))
	{
		auto FeaturePanel = Cast<UBuildGridMapFeaturePanel>(CurrentFeaturePanels[InFeatureIndex]);
		if (FeaturePanel)
		{
			FeaturePanel->UpdateTokenProperty(InPropertyData);
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("[BuildGridMapTokenActorPanel.UpdateTokenProperty] FeaturePanel not found at index: %d"), InFeatureIndex);
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[BuildGridMapTokenActorPanel.UpdateTokenProperty] Index out of range: %d"), InFeatureIndex);
	}
}

void UBuildGridMapTokenActorPanel::UpdateTokenPropertyInArray(const int32 InFeatureIndex, const FName& InArrayName,
	const int32 InArrayIndex, const FSerializableTokenProperty& InPropertyData)
{
	auto CurrentFeaturePanels = FeatureRoot->GetAllChildren();
	if (CurrentFeaturePanels.IsValidIndex(InFeatureIndex))
	{
		auto FeaturePanel = Cast<UBuildGridMapFeaturePanel>(CurrentFeaturePanels[InFeatureIndex]);
		if (FeaturePanel)
		{
			FeaturePanel->UpdateTokenPropertyInArray(InArrayName,InArrayIndex,InPropertyData);
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("[BuildGridMapTokenActorPanel.UpdateTokenPropertyInArray] FeaturePanel not found at index: %d"), InFeatureIndex);
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[BuildGridMapTokenActorPanel.UpdateTokenPropertyInArray] Index out of range: %d"), InFeatureIndex);
	}
}

void UBuildGridMapTokenActorPanel::Clean()
{
	// 移除全部监听
	OnTokenActorTypeChanged.Clear();
	OnTokenFeaturePropertyChanged.Clear();
	OnTokenDeleteClicked.Clear();
	OnAddTokenPropertyArraySigniture.Clear();
	OnTokenDeletePropertyArray.Clear();
	OnPropertyArrayValueChanged.Clear();
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

void UBuildGridMapTokenActorPanel::OnFeaturePropertyChange(int InFeatureIndex,const FName& PropertyName,
	const FString& PropertyValue)
{
	OnTokenFeaturePropertyChanged.Broadcast(TokenActorIndex, InFeatureIndex, PropertyName, PropertyValue);
}

void UBuildGridMapTokenActorPanel::OnAddTokenPropertyArray(const int32 FeatureIndex,const FName& InPropertyArrayName)
{
	OnAddTokenPropertyArraySigniture.Broadcast(TokenActorIndex, FeatureIndex, InPropertyArrayName);
}

void UBuildGridMapTokenActorPanel::OnDeleteTokenPropertyArray(const int32 InFeatureIndex,
	const FName& InPropertyArrayName, const int32 InArrayIndex)
{
	OnTokenDeletePropertyArray.Broadcast(TokenActorIndex, InFeatureIndex, InPropertyArrayName, InArrayIndex);
}

void UBuildGridMapTokenActorPanel::OnTokenPropertyArrayValueChanged(const int32 InFeatureIndex,
	const FName& InPropertyArrayName, const int32 InArrayIndex, const FName& PropertyName, const FString& NewValue)
{
	OnPropertyArrayValueChanged.Broadcast(TokenActorIndex,InFeatureIndex,InPropertyArrayName,InArrayIndex,PropertyName,NewValue);
}


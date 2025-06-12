// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapFeaturePanel.h"

#include "BuildGridMap/UI/BuildGridMapPropertyLine.h"
#include "Components/VerticalBox.h"

void UBuildGridMapFeaturePanel::BindFeatureData(int32 InFeatureIndex, const FSerializableTokenFeature& InFeatureData)
{
	FeatureIndex = InFeatureIndex;
	SetFeatureName(InFeatureData.FeatureClass->GetName());
	for (const auto& Property : InFeatureData.Properties)
	{
		auto PropertyLine = CreateWidget<UBuildGridMapPropertyLine>(this, PropertyLineClass);
		PropertyLine->InitPropertyLine(Property);
		PropertyLine->OnPropertyValueChanged.AddDynamic(this, &UBuildGridMapFeaturePanel::OnPropertyLineChanged);
		PropertiesRoot->AddChildToVerticalBox(PropertyLine);
	}
}

void UBuildGridMapFeaturePanel::Clean()
{
	OnFeaturePropertyChanged.Clear();
	auto Children = PropertiesRoot->GetAllChildren();
	for (auto Child : Children)
	{
		auto PropertyLine = Cast<UBuildGridMapPropertyLine>(Child);
		if (!PropertyLine)
		{
			continue;
		}
		PropertyLine->Clean();
	}
}

void UBuildGridMapFeaturePanel::OnPropertyLineChanged(const FName& PropertyName, const FString& NewValue)
{
	OnFeaturePropertyChanged.Broadcast(FeatureIndex, PropertyName, NewValue);
}

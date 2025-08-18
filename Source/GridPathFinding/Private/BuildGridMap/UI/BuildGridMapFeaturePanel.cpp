// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapFeaturePanel.h"

#include "GridPathFinding.h"
#include "BuildGridMap/UI/BuildGridMapPropertyLine.h"
#include "Components/VerticalBox.h"

void UBuildGridMapFeaturePanel::BindFeatureData(int32 InFeatureIndex, const FSerializableTokenFeature& InFeatureData)
{
	FeatureIndex = InFeatureIndex;
	SetFeatureName(InFeatureData.FeatureClass->GetName());
	for (const auto& Property : InFeatureData.Properties)
	{
		/*for (const auto& PropertyLine : Property.PropertyMap)
		{
			auto PropertyLineWidget = CreateWidget<UBuildGridMapPropertyLine>(this, PropertyLineClass);
			PropertyLineWidget->InitPropertyLine(PropertyLine);
			PropertyLineWidget->OnPropertyValueChanged.AddDynamic(this, &UBuildGridMapFeaturePanel::OnPropertyLineChanged);
			PropertiesRoot->AddChildToVerticalBox(PropertyLineWidget);
		}*/
		auto PropertyLine = CreateWidget<UBuildGridMapPropertyLine>(this, PropertyLineClass);
		PropertyLine->InitPropertyLine(Property);
		PropertyLine->OnPropertyValueChanged.AddDynamic(this, &UBuildGridMapFeaturePanel::OnPropertyLineChanged);
		PropertiesRoot->AddChildToVerticalBox(PropertyLine);
	}

	int32 Index = 0;
	for (const auto& PropertyArray : InFeatureData.PropertiesArray)
	{
		auto ArrayPanel = CreateWidget<UBuildGridMapPropertyArrayPanel>(this, PropertyArrayPanelClass);
		ArrayPanel->BindPropertiesArrayData(Index,PropertyArray);
		ArrayPanel->OnAddPropertyArrayDelegate.AddUObject(this, &UBuildGridMapFeaturePanel::AddTokenPropertyAraay);
		ArrayPanel->OnDeletePropertyArrayDelegate.AddDynamic(this, &UBuildGridMapFeaturePanel::OnDeleteTokenPropertyArray);
		ArrayPanel->OnChangePropertyArrayValueSignature.AddDynamic(this,&UBuildGridMapFeaturePanel::PropertyArrayValueChanged);
		PropertiesRoot->AddChildToVerticalBox(ArrayPanel);
		++Index;
	}
	/*for (const auto& PropertyPair : InFeatureData.PropertiesArrayMap)
	{
		for (const auto& Properties : PropertyPair.Value.PropertyArray)
		{
			for (const auto& Property : Properties)
			{
				auto PropertyLine = CreateWidget<UBuildGridMapPropertyLine>(this, PropertyLineClass);
				PropertyLine->InitPropertyLine(Property);
				PropertyLine->OnPropertyValueChanged.AddDynamic(this, &UBuildGridMapFeaturePanel::OnPropertyLineChanged);
				PropertiesRoot->AddChildToVerticalBox(PropertyLine);
			}
		}
	}*/
}

void UBuildGridMapFeaturePanel::Clean()
{
	OnFeaturePropertyChanged.Clear();
	OnPropertyArrayValueChangedSignature.Clear();
	auto Children = PropertiesRoot->GetAllChildren();
	for (auto Child : Children)
	{
		if (auto PropertyLine = Cast<UBuildGridMapPropertyLine>(Child))
		{
			PropertyLine->Clean();
		}
		else if (auto PropertyArrayPanel = Cast<UBuildGridMapPropertyArrayPanel>(Child)) 
		{
			PropertyArrayPanel->Clean();
		}
	}
}

void UBuildGridMapFeaturePanel::UpdateTokenPropertyArray(const FName& InArrayName,const FSerializableTokenPropertyArray& InPropertyArrayData)
{
	auto CurrentProperties = PropertiesRoot->GetAllChildren();
	for (auto Child : CurrentProperties)
	{
		if (UBuildGridMapPropertyArrayPanel* ArrayPanel = Cast<UBuildGridMapPropertyArrayPanel>(Child))
		{
			if (ArrayPanel->GetArrayName() == InArrayName)
			{
				ArrayPanel->UpdatePropertyArray(InPropertyArrayData);
			}
		}
	}
}

void UBuildGridMapFeaturePanel::UpdateTokenProperty(const FSerializableTokenProperty& InPropertyData)
{
	auto CurrentProperties = PropertiesRoot->GetAllChildren();
	for (auto Child : CurrentProperties)
	{
		if (UBuildGridMapPropertyLine* PropertyLine = Cast<UBuildGridMapPropertyLine>(Child))
		{
			if (PropertyLine->GetPropertyName() == InPropertyData.PropertyName)
			{
				PropertyLine->UpdateProperty(InPropertyData);
			}
		}
	}
}

void UBuildGridMapFeaturePanel::UpdateTokenPropertyInArray(const FName& InArrayName, const int32 InArrayIndex,
	const FSerializableTokenProperty& InPropertyData)
{
	auto CurrentProperties = PropertiesRoot->GetAllChildren();
	for (auto Child : CurrentProperties)
	{
		if (UBuildGridMapPropertyArrayPanel* ArrayPanel = Cast<UBuildGridMapPropertyArrayPanel>(Child))
		{
			if (ArrayPanel->GetArrayName() == InArrayName)
			{
				ArrayPanel->UpdatePropertyInArray(InArrayIndex,InPropertyData);
			}
		}
	}
}

void UBuildGridMapFeaturePanel::OnPropertyLineChanged(const FName& PropertyName, const FString& NewValue)
{
	OnFeaturePropertyChanged.Broadcast(FeatureIndex, PropertyName, NewValue);
}

void UBuildGridMapFeaturePanel::AddTokenPropertyAraay(const FName& PropertyArrayName)
{
	OnAddTokenPropertyArrayDelegate.Broadcast(FeatureIndex, PropertyArrayName);
}

void UBuildGridMapFeaturePanel::OnDeleteTokenPropertyArray(const FName& InPropertyArrayName, const int32 InArrayIndex)
{
	OnDeletePropertyArraySignature.Broadcast(FeatureIndex, InPropertyArrayName, InArrayIndex);
}

void UBuildGridMapFeaturePanel::PropertyArrayValueChanged(const FName& ArrayName, const int32 InArrayIndex,
	const FName& PropertyName, const FString& NewValue)
{
	OnPropertyArrayValueChangedSignature.Broadcast(FeatureIndex,ArrayName,InArrayIndex,PropertyName,NewValue);
}

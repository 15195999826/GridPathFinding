// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapPropertyArrayPanel.h"

#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Components/VerticalBox.h"

void UBuildGridMapPropertyArrayPanel::AddPropertyArrayButtonClick()
{
	OnAddPropertyArrayDelegate.Broadcast(ArrayName);
}

void UBuildGridMapPropertyArrayPanel::DeletePropertyArrayButtonClick(const int32 InArrayIndex)
{
	OnDeletePropertyArrayDelegate.Broadcast(ArrayName, InArrayIndex);
}

void UBuildGridMapPropertyArrayPanel::PropertyArrayValueChanged(const int32 InArrayIndex, const FName& PropertyName,
	const FString& NewValue)
{
	OnChangePropertyArrayValueSignature.Broadcast(ArrayName, InArrayIndex, PropertyName, NewValue);
}

void UBuildGridMapPropertyArrayPanel::BindPropertiesArrayData(int32 InPropertiesArrayIndex,
                                                              const FSerializableTokenPropertyArray& InPropertiesArrayData)
{
	SetArrayName(InPropertiesArrayData.PropertyArrayName.ToString());
	ArrayName = InPropertiesArrayData.PropertyArrayName;
	ArrayIndex = InPropertiesArrayIndex;
	AddPropertiesButton->OnClicked.AddDynamic(this,&UBuildGridMapPropertyArrayPanel::AddPropertyArrayButtonClick);
	int32 Index = 0;
	for (const auto& Properties : InPropertiesArrayData.PropertyArray)
	{
		auto PropertyLineArray = CreateWidget<UBuildGridMapPropertyLineArray>(this, PropertyLineArrayClass);
		PropertyLineArray->BindPropertiesArrayData(Index, Properties);
		PropertyLineArray->OnChangePropertyArrayValueDelegate.AddDynamic(this,&UBuildGridMapPropertyArrayPanel::PropertyArrayValueChanged);
		PropertyLineArray->OnTokenDeletePropertyArrayDelegate.AddDynamic(this,&UBuildGridMapPropertyArrayPanel::DeletePropertyArrayButtonClick);
		PropertiesArrayRoot->AddChildToVerticalBox(PropertyLineArray);
		++Index;
	}
}

void UBuildGridMapPropertyArrayPanel::Clean()
{
	auto Children = PropertiesArrayRoot->GetAllChildren();
	for (auto Child : Children)
	{
		auto PropertyLineArray = Cast<UBuildGridMapPropertyLineArray>(Child);
		if (!PropertyLineArray)
		{
			continue;
		}
		PropertyLineArray->Clean();
	}
	AddPropertiesButton->OnClicked.Clear();
	OnAddPropertyArrayDelegate.Clear();
	OnDeletePropertyArrayDelegate.Clear();
	OnChangePropertyArrayValueSignature.Clear();
}

void UBuildGridMapPropertyArrayPanel::UpdatePropertyArray(const FSerializableTokenPropertyArray& InPropertyArrayData)
{
	auto Children = PropertiesArrayRoot->GetAllChildren();
	for (auto Child : Children)
	{
		auto PropertyLineArray = Cast<UBuildGridMapPropertyLineArray>(Child);
		if (!PropertyLineArray)
		{
			continue;
		}
		PropertyLineArray->Clean();
	}
	PropertiesArrayRoot->ClearChildren();
	int32 Index = 0;
	for (const auto& Properties : InPropertyArrayData.PropertyArray)
	{
		auto PropertyLineArray = CreateWidget<UBuildGridMapPropertyLineArray>(this, PropertyLineArrayClass);
		PropertyLineArray->BindPropertiesArrayData(Index, Properties);
		PropertyLineArray->OnChangePropertyArrayValueDelegate.AddDynamic(this,&UBuildGridMapPropertyArrayPanel::PropertyArrayValueChanged);
		PropertyLineArray->OnTokenDeletePropertyArrayDelegate.AddDynamic(this,&UBuildGridMapPropertyArrayPanel::DeletePropertyArrayButtonClick);
		PropertiesArrayRoot->AddChildToVerticalBox(PropertyLineArray);
		++Index;
	}
}

void UBuildGridMapPropertyArrayPanel::UpdatePropertyInArray(const int32 InArrayIndex,
	const FSerializableTokenProperty& InPropertyData)
{
	auto CurrentProperties = PropertiesArrayRoot->GetAllChildren();
	Cast<UBuildGridMapPropertyLineArray>(CurrentProperties[InArrayIndex])->UpdatePropertyInArray(InPropertyData);
}




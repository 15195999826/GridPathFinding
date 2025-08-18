// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapPropertyLineArray.h"


#include "Components/VerticalBox.h"

void UBuildGridMapPropertyLineArray::BindPropertiesArrayData(int32 InPropertyArrayIndex,
                                                             const TArray<FSerializableTokenProperty>& InPropertyArrayData)
{
	ArrayIndex = InPropertyArrayIndex;
	SetPropertyArrayName();
	for (const auto& Property : InPropertyArrayData)
	{
		auto PropertyLine = CreateWidget<UBuildGridMapPropertyLine>(this, PropertyLineClass);
		FSerializableTokenProperty NewProperty(Property.PropertyType, Property.PropertyName, Property.Value);
		PropertyLine->InitPropertyLine(NewProperty);
		PropertyLine->OnPropertyValueChanged.AddDynamic(this,&UBuildGridMapPropertyLineArray::PropertyValueChanged);
		PropertiesRoot->AddChildToVerticalBox(PropertyLine);
	}
}

void UBuildGridMapPropertyLineArray::Clean()
{
	OnTokenDeletePropertyArrayDelegate.Clear();
	OnChangePropertyArrayValueDelegate.Clear();
}

void UBuildGridMapPropertyLineArray::UpdatePropertyInArray(const FSerializableTokenProperty& InPropertyData)
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

void UBuildGridMapPropertyLineArray::DeletePropertyArrayButtonClick()
{
	OnTokenDeletePropertyArrayDelegate.Broadcast(ArrayIndex);
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
	Clean();
}

void UBuildGridMapPropertyLineArray::PropertyValueChanged(const FName& PropertyName, const FString& NewValue)
{
	OnChangePropertyArrayValueDelegate.Broadcast(ArrayIndex, PropertyName, NewValue);
}


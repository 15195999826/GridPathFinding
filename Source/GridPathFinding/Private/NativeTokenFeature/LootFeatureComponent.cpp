// Fill out your copyright notice in the Description page of Project Settings.


#include "NativeTokenFeature/LootFeatureComponent.h"

const FName ULootFeatureComponent::LootDropRowPropertyName = TEXT("Row");
const FName ULootFeatureComponent::LootDropCountPropertyName = TEXT("Count");
const FName ULootFeatureComponent::LootDropProbabilityName = TEXT("Probability");
const FName ULootFeatureComponent::LootDropName = TEXT("ArrayDropName");
const FName ULootFeatureComponent::LootTestName = TEXT("ArrayTestName");
const FName ULootFeatureComponent::LootDropTestRowPropertyName = TEXT("DropTestRowName");
const FName ULootFeatureComponent::LootDropTestValuePropertyName = TEXT("DropTestValueName");
const FName ULootFeatureComponent::Array2Name = TEXT("Array2Name");
const FName ULootFeatureComponent::Array2Value = TEXT("Array2Value");

// Sets default values for this component's properties
ULootFeatureComponent::ULootFeatureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	FLootDrop Drop1(TEXT("1"), TEXT("3"), TEXT("50"));
	FLootDrop Drop2(TEXT("2"), TEXT("5"), TEXT("100"));
	LootDropData.Add(Drop1);
	LootDropData.Add(Drop2);

	FLootTest TestData1("Text1", "Value1");
	FLootTest TestData2("Text2", "Value2");
	LootTestData.Add(TestData1);
	LootTestData.Add(TestData2);
}

FSerializableTokenFeature ULootFeatureComponent::SerializeFeatureProperties() const
{
	FSerializableTokenFeature TokenFeature;
	TokenFeature.FeatureClass = ULootFeatureComponent::StaticClass();
	FSerializableTokenPropertyArray DropProperties;
	DropProperties.PropertyArrayName = LootDropName;
	FSerializableTokenPropertyArray TestProperties;
	TestProperties.PropertyArrayName = LootTestName;
	for (const auto& LootDrop : LootDropData)
	{
		TArray<FSerializableTokenProperty> PropertyArray;
		
		FSerializableTokenProperty NameProperty;
		NameProperty.PropertyName = LootDropRowPropertyName;
		NameProperty.PropertyType = ETokenPropertyType::String;
		NameProperty.Value = LootDrop.RowName;
		PropertyArray.Add(NameProperty);

		FSerializableTokenProperty TypeProperty;
		TypeProperty.PropertyName = LootDropCountPropertyName;
		TypeProperty.PropertyType = ETokenPropertyType::String;
		TypeProperty.Value = LootDrop.Count;
		PropertyArray.Add(TypeProperty);
		
		FSerializableTokenProperty ValueProperty;
		ValueProperty.PropertyName = LootDropProbabilityName;
		ValueProperty.PropertyType = ETokenPropertyType::String;
		ValueProperty.Value = LootDrop.Probability;
		PropertyArray.Add(ValueProperty);

		DropProperties.PropertyArray.Add(PropertyArray);
	}

	for (const auto& Test : LootTestData)
	{
		TArray<FSerializableTokenProperty> PropertyArray;
		
		FSerializableTokenProperty NameProperty;
		NameProperty.PropertyName = Array2Name;
		NameProperty.PropertyType = ETokenPropertyType::String;
		NameProperty.Value = Test.RowName;
		PropertyArray.Add(NameProperty);

		FSerializableTokenProperty TypeProperty;
		TypeProperty.PropertyName = Array2Value;
		TypeProperty.PropertyType = ETokenPropertyType::String;
		TypeProperty.Value = Test.Value;
		PropertyArray.Add(TypeProperty);
		
		TestProperties.PropertyArray.Add(PropertyArray);
	}
	
	TokenFeature.PropertiesArray.Add( DropProperties);
	TokenFeature.PropertiesArray.Add( TestProperties);

	FSerializableTokenProperty PropertyTest1(ETokenPropertyType::String,LootDropTestRowPropertyName,Test1);
	TokenFeature.Properties.Add(PropertyTest1);

	FSerializableTokenProperty PropertyTest2(ETokenPropertyType::String,LootDropTestValuePropertyName,Test2);
	TokenFeature.Properties.Add(PropertyTest2);
	
	return TokenFeature;
}

void ULootFeatureComponent::DeserializeFeatureProperties(const FSerializableTokenFeature& TokenFeature)
{
	for (const auto& Property : TokenFeature.Properties)
	{
		UpdateFeatureProperty(Property);
	}
	for (const auto& PropertiesArray : TokenFeature.PropertiesArray)
	{
		int32 Index = 0;
		if (PropertiesArray.PropertyArrayName == LootDropName)
		{
			LootDropData.SetNum(PropertiesArray.PropertyArray.Num());
		}
		else if (PropertiesArray.PropertyArrayName == LootTestName)
		{
			LootTestData.SetNum(PropertiesArray.PropertyArray.Num());
		}
		for (const auto& Properties : PropertiesArray.PropertyArray)
		{
			UpdateFeaturePropertyArray(Properties,PropertiesArray.PropertyArrayName,Index);
			Index++;
		}
	}
}

void ULootFeatureComponent::UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty)
{
	if (InNewProperty.PropertyName == LootDropTestRowPropertyName)
	{
		Test1 = InNewProperty.Value;
	}
	else if (InNewProperty.PropertyName == LootDropTestValuePropertyName)
	{
		Test2 = InNewProperty.Value;
	}
}

void ULootFeatureComponent::UpdateFeaturePropertyArray(const TArray<FSerializableTokenProperty>& InNewPropertyArray,const FName& PropertyArrayName,const int32 UpdateIndex)
{
	if (PropertyArrayName == LootDropName)
	{
		for (auto& Property : InNewPropertyArray)
		{
			if (Property.PropertyName == LootDropRowPropertyName)
			{
				LootDropData[UpdateIndex].RowName = Property.Value;
			}
			else if (Property.PropertyName == LootDropCountPropertyName)
			{
				LootDropData[UpdateIndex].Count = Property.Value;
			}
			else if (Property.PropertyName == LootDropProbabilityName)
			{
				LootDropData[UpdateIndex].Probability = Property.Value;
			}
		}
	}
	else if (PropertyArrayName == LootTestName)
	{
		for (auto& Property : InNewPropertyArray)
		{
			if (Property.PropertyName == Array2Name)
			{
				LootTestData[UpdateIndex].RowName = Property.Value;
			}
			else if (Property.PropertyName == Array2Value)
			{
				LootTestData[UpdateIndex].Value = Property.Value;
			}
		}
	}
}

TArray<FSerializableTokenProperty> ULootFeatureComponent::CreatePropertyArray(const FName& PropertyArrayName)
{
	TArray<FSerializableTokenProperty>	 PropertyArray;
	if (PropertyArrayName == LootDropName)
	{
		FSerializableTokenProperty RowProperty(ETokenPropertyType::String, LootDropRowPropertyName, TEXT("None"));
		FSerializableTokenProperty CountProperty(ETokenPropertyType::String, LootDropCountPropertyName, TEXT("1"));
		FSerializableTokenProperty ProbabilityProperty(ETokenPropertyType::String, LootDropProbabilityName, TEXT("0"));
		PropertyArray.Add(RowProperty);
		PropertyArray.Add(CountProperty);
		PropertyArray.Add(ProbabilityProperty);
	}
	else if (PropertyArrayName == LootTestName)
	{
		FSerializableTokenProperty RowProperty(ETokenPropertyType::String, Array2Name, TEXT("None"));
		FSerializableTokenProperty ValueProperty(ETokenPropertyType::String, Array2Value, TEXT("1"));
		PropertyArray.Add(RowProperty);
		PropertyArray.Add(ValueProperty);
	}
	return PropertyArray;
}



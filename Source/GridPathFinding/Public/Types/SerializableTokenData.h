#pragma once
#include "CoreMinimal.h"
#include "UObject/ReferenceChainSearch.h"

#include "SerializableTokenData.generated.h"

class ATokenActor;

// Todo: 暂时不支持数组
UENUM()
enum class ETokenPropertyType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Float = 1 UMETA(DisplayName = "Float"),
	Int = 2 UMETA(DisplayName = "Int"),
	String = 3 UMETA(DisplayName = "String"),
	Bool = 4 UMETA(DisplayName = "Bool"),
	Vector = 5 UMETA(DisplayName = "Vector"),
	SoftMeshPath = 6 UMETA(DisplayName = "SoftMesh"),
};


USTRUCT(BlueprintType)
struct FSerializableTokenProperty
{
	GENERATED_BODY()

	FSerializableTokenProperty()
	{
	}

	FSerializableTokenProperty(ETokenPropertyType InPropertyType, const FName& InPropertyName, const FString& InValue)
		: PropertyType(InPropertyType), PropertyName(InPropertyName), Value(InValue)
	{
	}

	UPROPERTY()
	ETokenPropertyType PropertyType{ETokenPropertyType::None};

	UPROPERTY()
	FName PropertyName;

	UPROPERTY()
	FString Value;
};

USTRUCT(BlueprintType)
struct FSerializableTokenPropertyArray
{
	GENERATED_BODY()

	FSerializableTokenPropertyArray()
	{
	};

	FName PropertyArrayName;

	TArray<TArray<FSerializableTokenProperty>> PropertyArray;

	const FSerializableTokenProperty* FindPropertyByPropertyName(const int InArrayIndex, const FName& InPropertyName) const
	{
		for (const FSerializableTokenProperty& Property : PropertyArray[InArrayIndex])
		{
			if (Property.PropertyName == InPropertyName)
			{
				return &Property;
			}
		}
		return nullptr;
	}
	
	FSerializableTokenProperty* FindMutPropertyByPropertyName(const int InArrayIndex, const FName& InPropertyName)
	{
		for (FSerializableTokenProperty& Property : PropertyArray[InArrayIndex])
		{
			if (Property.PropertyName == InPropertyName)
			{
				return &Property;
			}
		}
		return nullptr;
	}
};

USTRUCT(BlueprintType)
struct FSerializableTokenFeature
{
	GENERATED_BODY()

	FSerializableTokenFeature()
	{
	}

	UPROPERTY()
	TSubclassOf<UActorComponent> FeatureClass;

	UPROPERTY()
	TArray<FSerializableTokenProperty> Properties;

	UPROPERTY()
	TArray<FSerializableTokenPropertyArray> PropertiesArray;


	const FSerializableTokenProperty* FindPropertyByPropertyName(const FName& InPropertyName) const
	{
		for (const FSerializableTokenProperty& Property : Properties)
		{
			if (Property.PropertyName == InPropertyName)
			{
				return &Property;
			}
		}
		return nullptr;
	}

	FSerializableTokenProperty* FindMutPropertyByPropertyName(const FName& InPropertyName)
	{
		for (FSerializableTokenProperty& Property : Properties)
		{
			if (Property.PropertyName == InPropertyName)
			{
				return &Property;
			}
		}
		return nullptr;
	}

	const FSerializableTokenPropertyArray* FindPropertyArrayByArrayName(const FName& InPropertyArrayName) const 
	{
		for (const FSerializableTokenPropertyArray& Property : PropertiesArray)
		{
			if (Property.PropertyArrayName == InPropertyArrayName)
			{
				return &Property;
			}
		}
		return nullptr;
	}
	
	FSerializableTokenPropertyArray* FindMutPropertyArrayByArrayName(const FName& InPropertyArrayName)
	{
		for (FSerializableTokenPropertyArray& Property : PropertiesArray)
		{
			if (Property.PropertyArrayName == InPropertyArrayName)
			{
				return &Property;
			}
		}
		return nullptr;
	}
};


USTRUCT(BlueprintType)
struct FSerializableTokenData
{
	GENERATED_BODY()

	FSerializableTokenData()
	{
	}

	UPROPERTY()
	TSubclassOf<ATokenActor> TokenClass;

	UPROPERTY()
	TArray<FSerializableTokenFeature> Features;
};

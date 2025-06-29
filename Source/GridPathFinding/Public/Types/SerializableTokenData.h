#pragma once
#include "CoreMinimal.h"

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

	FSerializableTokenProperty() {}

	FSerializableTokenProperty(ETokenPropertyType InPropertyType, const FName& InPropertyName, const FString& InValue)
		: PropertyType(InPropertyType), PropertyName(InPropertyName), Value(InValue) {}

	UPROPERTY()
	ETokenPropertyType PropertyType{ETokenPropertyType::None};
	
	UPROPERTY()
	FName PropertyName;

	UPROPERTY()
	FString Value;
};

USTRUCT(BlueprintType)
struct FSerializableTokenFeature
{
	GENERATED_BODY()

	FSerializableTokenFeature() {}

	UPROPERTY()
	TSubclassOf<UActorComponent> FeatureClass;

	UPROPERTY()
	TArray<FSerializableTokenProperty> Properties;
};

USTRUCT(BlueprintType)
struct FSerializableTokenData
{
	GENERATED_BODY()

	FSerializableTokenData(){}
	
	UPROPERTY()
	TSubclassOf<ATokenActor> TokenClass;

	UPROPERTY()
	TArray<FSerializableTokenFeature> Features;
};

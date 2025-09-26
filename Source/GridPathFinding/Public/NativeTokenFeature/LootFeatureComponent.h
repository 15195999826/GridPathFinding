// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TokenFeatureInterface.h"
#include "LootFeatureComponent.generated.h"

USTRUCT()
struct FLootDrop
{
	GENERATED_BODY()

	FLootDrop() : RowName("None"), Count("0"), Probability("0") {}
	FLootDrop(const FString& InRowName,const FString& InCount,const FString& InProbability):RowName(InRowName),Count(InCount),Probability(InProbability)
	{
		
	};

	UPROPERTY()
	FString RowName;

	UPROPERTY()
	FString Count;

	UPROPERTY()
	FString Probability;
};

USTRUCT()
struct FLootTest
{
	GENERATED_BODY()
	FLootTest() : RowName("None"), Value("0") {}
	FLootTest(const FString& InRowName,const FString& InValue):RowName(InRowName), Value(InValue)
	{
		
	};

	UPROPERTY()
	FString RowName;

	UPROPERTY()
	FString Value;
	
};

UCLASS(Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDPATHFINDING_API ULootFeatureComponent : public UActorComponent, public ITokenFeatureInterface
{
	GENERATED_BODY()

	static const FName LootDropRowPropertyName;
	static const FName LootDropCountPropertyName;
	static const FName LootDropProbabilityName;

	static const FName LootDropName;
	static const FName LootTestName;

	static const FName LootDropTestRowPropertyName;
	static const FName LootDropTestValuePropertyName;

	static const FName Array2Name;
	static const FName Array2Value;

public:
	
	ULootFeatureComponent();
	
	virtual FSerializableTokenFeature SerializeFeatureProperties() const override;

	virtual void DeserializeFeatureProperties(const FSerializableTokenFeature& TokenFeature) override;

	virtual void UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty) override;

	virtual void UpdateFeaturePropertyArray(const TArray<FSerializableTokenProperty>& InNewPropertyArray,const FName& PropertyArrayName,const int32 UpdateIndex) override;
	
	virtual void InitGameplayFeature(UGridMapModel* MapModel) override{};

	virtual TArray<FSerializableTokenProperty> CreatePropertyArray(const FName& PropertyArrayName) override;

	FString Test1 = "Test1";

	FString Test2 = "Test2";
	
	TArray<FLootDrop> LootDropData;

	TArray<FLootTest> LootTestData;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SerializableTokenData.h"
#include "Types/TokenActorData.h"
#include "TokenActor.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTokenActorRemoveFromMapDelegate, ATokenActor* /*TokenActor*/);

UCLASS()
class GRIDPATHFINDING_API ATokenActor : public AActor
{
	GENERATED_BODY()

	inline static int32 TokenIDCounter = 0;

public:
	// Sets default values for this actor's properties
	ATokenActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	int32 GetTokenID() const
	{
		return TokenID;
	}

	/**
	 * 游戏运行时， 初始化Token功能, 需要自行在游戏玩法流程中调用
	 */
	void InitGameplayToken() const;

	FOnTokenActorRemoveFromMapDelegate OnRemoveFromMap;

	FSerializableTokenData SerializableTokenData();

	void DeserializeTokenData(const FSerializableTokenData& TokenData);

	void DeserializeFeatureData(const int32 FeatureIndex,const FSerializableTokenFeature& FeatureData);
	
	void UpdateFeatureProperty(int InFeatureIndex, TSubclassOf<UActorComponent> FeatureClass, const FSerializableTokenProperty& PropertyCopy);

	void UpdatePropertyArray(const int InFeatureIndex,const FName& PropertyArrayName, const int ArrayIndex ,TSubclassOf<UActorComponent> FeatureClass, const TArray<FSerializableTokenProperty>& PropertyCopy);
	
	void SetCustomGameplayData(const FName& Key, const FString& Value)
	{
		CustomGameplayData.Add(Key, Value);
	}

	FString GetCustomGameplayData(const FName& Key) const
	{
		if (CustomGameplayData.Contains(Key))
		{
			return CustomGameplayData[Key];
		}
		return FString();
	}

protected:
	UPROPERTY()
	TMap<FName, FString> CustomGameplayData; // 自定义游戏数据
	
private:
	int32 TokenID;


#pragma region Serialization
public:
	// 将Actor转换为结构体
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	FTokenActorStruct ToStruct() const;

	// 从结构体更新Actor
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	bool FromStruct(const FTokenActorStruct& ActorStruct);

	// 将结构体序列化为JSON字符串
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	static FString StructToJson(const FTokenActorStruct& ActorStruct);

	// 从JSON字符串反序列化为结构体
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	static bool JsonToStruct(const FString& JsonString, FTokenActorStruct& OutActorStruct);

	// 将Actor序列化为JSON字符串
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	FString SerializeToJson() const;

	// 从JSON字符串反序列化Actor
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	bool DeserializeFromJson(const FString& JsonString);

	// 将Actor保存到JSON文件
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	bool SaveToJsonFile(const FString& FilePath) const;

	// 从JSON文件加载Actor
	UFUNCTION(BlueprintCallable, Category = "Serialization")
	bool LoadFromJsonFile(const FString& FilePath);
#pragma endregion
};



// Fill out your copyright notice in the Description page of Project Settings.


#include "TokenActor.h"

#include "GridPathFinding.h"
#include "JsonObjectConverter.h"
#include "TokenFeatureInterface.h"
#if WITH_EDITOR
#include "BuildGridMap/BuildGridMapGameMode.h"
#endif


// Sets default values
ATokenActor::ATokenActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);

	// 不使用ObjectPool管理，ID在构造时直接分配
	TokenID = TokenIDCounter++;
	Tags.Add("CanHit");
}

// Called when the game starts or when spawned
void ATokenActor::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITOR
	auto GM = GetWorld()->GetAuthGameMode();
	if (Cast<ABuildGridMapGameMode>(GM))
	{
		return;
	}
#endif

	MeshComponent->SetVisibility(false);
}

void ATokenActor::InitGameplayToken(UGridMapModel* MapModel) const
{
	auto TokenFeatures = GetComponentsByInterface(UTokenFeatureInterface::StaticClass());

	for (UActorComponent* Component : TokenFeatures)
	{
		auto FeatureInterface = Cast<ITokenFeatureInterface>(Component);
		FeatureInterface->InitGameplayFeature(MapModel);
	}
}

FSerializableTokenData ATokenActor::SerializableTokenData()
{
	FSerializableTokenData TokenData;
	TokenData.TokenClass = GetClass();
	// 获取全部实现了UTokenFeatureInterface的组件
	auto TokenFeatures = GetComponentsByInterface(UTokenFeatureInterface::StaticClass());

	for (UActorComponent* Component : TokenFeatures)
	{
		// 将组件转换为FSerializableTokenFeature
		FSerializableTokenFeature FeatureData;
		auto FeatureInterface = Cast<ITokenFeatureInterface>(Component);
		FeatureData = FeatureInterface->SerializeFeatureProperties();
		FeatureData.FeatureClass = Component->GetClass();
		TokenData.Features.Add(FeatureData);
	}
	
	return TokenData;
}

void ATokenActor::DeserializeTokenData(const FSerializableTokenData& TokenData)
{
	auto TokenFeatures = GetComponentsByInterface(UTokenFeatureInterface::StaticClass());

	for (UActorComponent* Component : TokenFeatures)
	{
		auto FeatureInterface = Cast<ITokenFeatureInterface>(Component);

		// Todo: 是否可以避免双循环检测的优化
		bool Find = false;
		for (const FSerializableTokenFeature& FeatureData : TokenData.Features)
		{
			if (Component->GetClass() == FeatureData.FeatureClass)
			{
				// 反序列化新的属性
				FeatureInterface->DeserializeFeatureProperties(FeatureData);
				Find = true;
				break;
			}
		}

		if (!Find)
		{
			// 如果没有找到对应的FeatureData， 可以选择清空属性或保留原有属性
			UE_LOG(LogGridPathFinding, Warning, TEXT("[ATokenActor.DeserializeTokenData] Feature %s not found in TokenData"), *Component->GetName());
		}
	}
}

void ATokenActor::DeserializeFeatureData(const int32 FeatureIndex, const FSerializableTokenFeature& FeatureData)
{
	auto TokenFeatures = GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
	
	auto FeatureInterface = Cast<ITokenFeatureInterface>(TokenFeatures[FeatureIndex]);
	FeatureInterface->DeserializeFeatureProperties(FeatureData);
	// Todo: 是否可以避免双循环检测的优化
	/*bool Find = false;
	for (const FSerializableTokenFeature& FeatureData : FeatureData)
	{
		if (TokenFeatures[FeatureIndex]->GetClass() == FeatureData.FeatureClass)
		{
			// 反序列化新的属性
			FeatureInterface->DeserializeFeatureProperties(FeatureData);
			Find = true;
			break;
		}
	}

	if (!Find)
	{
		// 如果没有找到对应的FeatureData， 可以选择清空属性或保留原有属性
		UE_LOG(LogGridPathFinding, Warning,
		       TEXT("[ATokenActor.DeserializeTokenData] Feature %s not found in TokenData"), *Component->GetName());
	}*/
}

void ATokenActor::UpdateFeatureProperty(int InFeatureIndex, TSubclassOf<UActorComponent> FeatureClass,
                                        const FSerializableTokenProperty& PropertyCopy)
{
	auto TokenFeatures = GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
	if (!TokenFeatures.IsValidIndex(InFeatureIndex))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ATokenActor.UpdateFeatureProperty] FeatureIndex %d is out of range, FeatureType: %s"),
			InFeatureIndex, *FeatureClass->GetName());
		return;
	}

	check(TokenFeatures[InFeatureIndex]->GetClass() == FeatureClass);
	auto FeatureInterface = Cast<ITokenFeatureInterface>(TokenFeatures[InFeatureIndex]);
	FeatureInterface->UpdateFeatureProperty(PropertyCopy);
}

void ATokenActor::UpdatePropertyArray(const int InFeatureIndex, const FName& PropertyArrayName, const int ArrayIndex,
	TSubclassOf<UActorComponent> FeatureClass, const TArray<FSerializableTokenProperty>& PropertyCopy)
{
	auto TokenFeatures = GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
	if (!TokenFeatures.IsValidIndex(InFeatureIndex))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ATokenActor.UpdateFeatureProperty] FeatureIndex %d is out of range, FeatureType: %s"),
			InFeatureIndex, *FeatureClass->GetName());
		return;
	}
	check(TokenFeatures[InFeatureIndex]->GetClass() == FeatureClass);
	auto FeatureInterface = Cast<ITokenFeatureInterface>(TokenFeatures[InFeatureIndex]);
	FeatureInterface->UpdateFeaturePropertyArray(PropertyCopy, PropertyArrayName, ArrayIndex);
}

FTokenActorStruct ATokenActor::ToStruct() const
{
	FTokenActorStruct ActorStruct;

	// 设置Actor基本信息
	ActorStruct.ActorName = GetName();
	ActorStruct.ActorClass = GetClass()->GetPathName();

	// 设置Actor变换
	ActorStruct.Transform = FTokenTransformStruct(GetActorTransform());

	// 设置SceneRoot组件信息
	// if (SceneRoot.IsValid())
	{
		ActorStruct.SceneRoot.ComponentName = SceneRoot->GetName();
		ActorStruct.SceneRoot.ComponentClass = SceneRoot->GetClass()->GetPathName();
		ActorStruct.SceneRoot.RelativeTransform = FTokenTransformStruct(SceneRoot->GetRelativeTransform());
	}

	// 设置MeshComponent组件信息
	// if (MeshComponent.IsValid())
	{
		ActorStruct.MeshComponent.ComponentName = MeshComponent->GetName();
		ActorStruct.MeshComponent.ComponentClass = MeshComponent->GetClass()->GetPathName();
		ActorStruct.MeshComponent.RelativeTransform = FTokenTransformStruct(MeshComponent->GetRelativeTransform());

		// 设置StaticMesh路径
		if (MeshComponent->GetStaticMesh())
		{
			ActorStruct.MeshComponent.MeshPath = MeshComponent->GetStaticMesh()->GetPathName();
		}

		// 设置材质路径
		for (int32 i = 0; i < MeshComponent->GetNumMaterials(); i++)
		{
			UMaterialInterface* Material = MeshComponent->GetMaterial(i);
			if (Material)
			{
				FTokenMaterialStruct MaterialStruct;
				MaterialStruct.MaterialPath = Material->GetPathName();
				ActorStruct.MeshComponent.Materials.Add(MaterialStruct);
			}
		}
	}

	return ActorStruct;
}

bool ATokenActor::FromStruct(const FTokenActorStruct& ActorStruct)
{
	// 设置Actor变换
	SetActorTransform(ActorStruct.Transform.ToFTransform());

	// 更新SceneRoot组件
	// if (SceneRoot.IsValid())
	{
		SceneRoot->SetRelativeTransform(ActorStruct.SceneRoot.RelativeTransform.ToFTransform());
	}

	// 更新MeshComponent组件
	// if (MeshComponent.IsValid())
	{
		// 设置相对变换
		MeshComponent->SetRelativeTransform(ActorStruct.MeshComponent.RelativeTransform.ToFTransform());

		// 设置StaticMesh
		if (!ActorStruct.MeshComponent.MeshPath.IsEmpty())
		{
			UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, *ActorStruct.MeshComponent.MeshPath);
			if (StaticMesh)
			{
				MeshComponent->SetStaticMesh(StaticMesh);
			}
		}

		// 设置材质
		for (int32 i = 0; i < ActorStruct.MeshComponent.Materials.Num(); i++)
		{
			const FTokenMaterialStruct& MaterialStruct = ActorStruct.MeshComponent.Materials[i];
			if (!MaterialStruct.MaterialPath.IsEmpty())
			{
				UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialStruct.MaterialPath);
				if (Material)
				{
					MeshComponent->SetMaterial(i, Material);
				}
			}
		}
	}

	return true;
}

FString ATokenActor::StructToJson(const FTokenActorStruct& ActorStruct)
{
	FString Ret;
	FJsonObjectConverter::UStructToJsonObjectString(ActorStruct, Ret);
	return Ret;
}

bool ATokenActor::JsonToStruct(const FString& JsonString, FTokenActorStruct& OutActorStruct)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutActorStruct, 0, 0);
}

FString ATokenActor::SerializeToJson() const
{
	const FTokenActorStruct ActorStruct = ToStruct();
	return StructToJson(ActorStruct);
}

bool ATokenActor::DeserializeFromJson(const FString& JsonString)
{
	FTokenActorStruct ActorStruct;
	if (JsonToStruct(JsonString, ActorStruct))
	{
		return FromStruct(ActorStruct);
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[ATokenActor.DeserializeFromJson]"));
	return false;
}

bool ATokenActor::SaveToJsonFile(const FString& FilePath) const
{
	FString JsonString = SerializeToJson();
	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool ATokenActor::LoadFromJsonFile(const FString& FilePath)
{
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return DeserializeFromJson(JsonString);
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[ATokenActor.LoadFromJsonFile]"))
	return false;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapCopyPasteCommand.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"

void UBuildGridMapCopyPasteCommand::Initialize(const FHCubeCoord& InSourceCoord, TArray<FHCubeCoord>&& InTargetCoords,
                                               const FSerializableTile& InSourceTileData)
{
	SourceCoord = InSourceCoord;
	TargetCoords = MoveTemp(InTargetCoords);
	SourceTileData = InSourceTileData;

	if (TargetCoords.Num() > 0)
	{
		auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
		const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();
		
		// 保存原始目标地块数据，用于撤销操作
		OriginalTargetTileData.Reserve(TargetCoords.Num());
		NewlyCreatedTiles.Reserve(TargetCoords.Num());
		
		for (const FHCubeCoord& TargetCoord : TargetCoords)
		{
			if (EditingTiles.Contains(TargetCoord))
			{
				// 保存原有数据
				OriginalTargetTileData.Add(TargetCoord, EditingTiles[TargetCoord]);
			}
			else
			{
				// 记录这是新创建的地块
				NewlyCreatedTiles.Add(TargetCoord);
			}
		}
	}
}

bool UBuildGridMapCopyPasteCommand::Execute()
{
	if (TargetCoords.IsEmpty())
	{
		UE_LOG(LogGridPathFinding, Warning, TEXT("[UBuildGridMapCopyPasteCommand.Execute] Target coords is empty!"));
		return false;
	}

	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();

	// 对每个目标地块执行粘贴操作
	for (const FHCubeCoord& TargetCoord : TargetCoords)
	{
		// 跳过源地块自身
		if (TargetCoord == SourceCoord)
		{
			continue;
		}
		
		// 创建新的地块数据，复制源地块的所有属性
		FSerializableTile NewTileData = SourceTileData;
		// 但是坐标要设置为目标坐标
		NewTileData.Coord = TargetCoord;
		
		// 更新到编辑数据中
		GM->GetMutEditingTiles().Add(TargetCoord, NewTileData);
		
		// 更新渲染
		GM->GridMapModel->UpdateTileEnv(NewTileData);
		GM->GridMapModel->IntervalDeserializeTokens(TargetCoord, NewTileData.SerializableTokens);
		
		// 标记为脏数据
		GM->MarkEditingTilesDirty(TargetCoord);
		
		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridMapCopyPasteCommand.Execute] Pasted tile data to coord: %s"), *TargetCoord.ToString());
	}

	return true;
}

bool UBuildGridMapCopyPasteCommand::Undo()
{
	// Todo: 暂时不实现
	return true;
}

FString UBuildGridMapCopyPasteCommand::GetDescription() const
{
	return FString::Printf(TEXT("复制粘贴地块 (源: %s, 目标数量: %d)"), *SourceCoord.ToString(), TargetCoords.Num());
}

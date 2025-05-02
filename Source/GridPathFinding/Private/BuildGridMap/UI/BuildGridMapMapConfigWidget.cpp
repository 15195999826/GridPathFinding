// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapMapConfigWidget.h"

#include "GridPathFinding.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"

void UBuildGridMapMapConfigWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 定义可见性规则
	// 定义可见性规则
	VisibilityRules = {
		{
			EGridMapType::HEX_STANDARD, {
				{EGridMapDrawMode::BaseOnRadius, {GridRadiusTextBox, TileOrientationComboBox, MapRadiusTextBox}},
				{EGridMapDrawMode::BaseOnRowColumn, {GridRadiusTextBox, TileOrientationComboBox, MapRowTextBox, MapColumnTextBox}},
				{EGridMapDrawMode::BaseOnVolume, {GridRadiusTextBox, TileOrientationComboBox}}
			}
		},
		{
			EGridMapType::SQUARE_STANDARD, {
				{EGridMapDrawMode::BaseOnRadius, {GridRadiusTextBox, MapRadiusTextBox}},
				{EGridMapDrawMode::BaseOnRowColumn, {GridRadiusTextBox, MapRowTextBox, MapColumnTextBox}},
				{EGridMapDrawMode::BaseOnVolume, {GridRadiusTextBox}}
			}
		},
		{
			EGridMapType::RECTANGLE_STANDARD, {
				{EGridMapDrawMode::BaseOnRadius, {GridSizeXTextBox, GridSizeYTextBox, MapRadiusTextBox}},
				{
					EGridMapDrawMode::BaseOnRowColumn,
					{GridSizeXTextBox, GridSizeYTextBox, MapRowTextBox, MapColumnTextBox}
				},
				{EGridMapDrawMode::BaseOnVolume, {GridSizeXTextBox, GridSizeYTextBox}}
			}
		},
		{
			EGridMapType::RECTANGLE_SIX_DIRECTION, {
				{EGridMapDrawMode::BaseOnRadius, {GridSizeXTextBox, GridSizeYTextBox, TileOrientationComboBox, MapRadiusTextBox}},
				{
					EGridMapDrawMode::BaseOnRowColumn,
					{GridSizeXTextBox, GridSizeYTextBox, TileOrientationComboBox, MapRowTextBox, MapColumnTextBox}
				},
				{EGridMapDrawMode::BaseOnVolume, {GridSizeXTextBox, GridSizeYTextBox, TileOrientationComboBox}}
			}
		}
	};

	// 初始化MapTypeComboBox的选项
	MapTypeComboBox->ClearOptions();
	// 针对 EGridMapType
	// 针对 EGridMapType
	UEnum* MapTypeEnum = StaticEnum<EGridMapType>();
	// 获取实际枚举数量（不含隐藏值）
	int32 EnumCount = MapTypeEnum->NumEnums() - 1; // -1 是为了排除枚举的 _MAX 类型值（如果存在）

	for (int32 i = 0; i < EnumCount; ++i)
	{
		FString EnumName = MapTypeEnum->GetDisplayNameTextByIndex(i).ToString();
		MapTypeComboBox->AddOption(EnumName);
	}

	// 针对 EGridMapDrawMode
	MapDrawModeComboBox->ClearOptions();
	UEnum* MapDrawModeEnum = StaticEnum<EGridMapDrawMode>();
	int32 DrawModeEnumCount = MapDrawModeEnum->NumEnums() - 1;
	for (int32 i = 0; i < DrawModeEnumCount; ++i)
	{
		FString EnumName = MapDrawModeEnum->GetDisplayNameTextByIndex(i).ToString();
		MapDrawModeComboBox->AddOption(EnumName);
	}

	// 针对 EHexTileOrientationFlag
	TileOrientationComboBox->ClearOptions();
	UEnum* HexTileOrientationEnum = StaticEnum<ETileOrientationFlag>();
	int32 HexTileOrientationEnumCount = HexTileOrientationEnum->NumEnums() - 1;
	for (int32 i = 0; i < HexTileOrientationEnumCount; ++i)
	{
		FString EnumName = HexTileOrientationEnum->GetDisplayNameTextByIndex(i).ToString();
		TileOrientationComboBox->AddOption(EnumName);
	}

	MapTypeComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridMapMapConfigWidget::OnMapTypeSelectionChanged);
	MapDrawModeComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridMapMapConfigWidget::OnMapDrawModeSelectionChanged);
	TileOrientationComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridMapMapConfigWidget::OnHexTileOrientationSelectionChanged);
}

void UBuildGridMapMapConfigWidget::SetMapConfig(const FName& InMapName, const FGridMapConfig& MapConfig)
{
	UE_LOG(LogGridPathFinding, Display, TEXT("[UBuildGridMapMapConfigWidget.SetMapConfig]"));
	// 设置地图名称
	MapNameTextBox->SetText(FText::FromName(InMapName));

	// 设置地图类型
	UEnum* MapTypeEnum = StaticEnum<EGridMapType>();
	int64 MapTypeValue = static_cast<int64>(MapConfig.MapType);
	FString MapTypeName = MapTypeEnum->GetDisplayNameTextByValue(MapTypeValue).ToString();
	MapTypeComboBox->SetSelectedOption(MapTypeName);

	// 设置地图绘制模式
	UEnum* MapDrawModeEnum = StaticEnum<EGridMapDrawMode>();
	int64 MapDrawModeValue = static_cast<int64>(MapConfig.DrawMode);
	FString MapDrawModeName = MapDrawModeEnum->GetDisplayNameTextByValue(MapDrawModeValue).ToString();
	MapDrawModeComboBox->SetSelectedOption(MapDrawModeName);

	// 设置六边形地图格子方向（仅当地图类型为六边形时适用）
	if (MapConfig.MapType == EGridMapType::HEX_STANDARD)
	{
		UEnum* HexOrientationEnum = StaticEnum<ETileOrientationFlag>();
		int64 HexOrientationValue = static_cast<int64>(MapConfig.TileOrientation);
		FString HexOrientationName = HexOrientationEnum->GetDisplayNameTextByValue(HexOrientationValue).ToString();
		TileOrientationComboBox->SetSelectedOption(HexOrientationName);
		// 显示六边形相关控件
		TileOrientationComboBox->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// 隐藏六边形相关控件
		TileOrientationComboBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 设置地图尺寸信息
	MapRowTextBox->SetText(FText::AsNumber(MapConfig.MapSize.X));
	MapColumnTextBox->SetText(FText::AsNumber(MapConfig.MapSize.Y));

	// 设置格子尺寸
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
		{
			GridRadiusTextBox->SetText(FText::AsNumber(MapConfig.HexGridRadius));
		}
		break;
	case EGridMapType::SQUARE_STANDARD:
		{
			GridRadiusTextBox->SetText(FText::AsNumber(MapConfig.SquareSize));
		}
		break;
	case EGridMapType::RECTANGLE_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			GridSizeXTextBox->SetText(FText::AsNumber(MapConfig.RectSize.X));
			GridSizeYTextBox->SetText(FText::AsNumber(MapConfig.RectSize.Y));
		}
		break;
	}
}

void UBuildGridMapMapConfigWidget::OnMapTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// 调整UI显示的内容
	// String To EGridMapType
	auto MapType = GetMapTypeFromString(SelectedItem);
	auto MapDrawMode = GetMapDrawModeFromString(MapDrawModeComboBox->GetSelectedOption());
	UpdateShowLine(MapType, MapDrawMode);

	// UI上改变选择时， 发送事件， 处理逻辑
	switch (SelectionType)
	{
	case ESelectInfo::OnMouseClick:
		{
			OnMapTypeChanged.Broadcast(MapType);
		}
		break;
	default:
		break;
	}
}

void UBuildGridMapMapConfigWidget::OnMapDrawModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// 调整UI显示的内容
	auto MapType = GetMapTypeFromString(MapTypeComboBox->GetSelectedOption());
	auto MapDrawMode = GetMapDrawModeFromString(SelectedItem);
	UpdateShowLine(MapType, MapDrawMode);

	switch (SelectionType)
	{
	case ESelectInfo::OnMouseClick:
		{
			OnMapDrawModeChanged.Broadcast(MapDrawMode);
		}
		break;
	default:
		break;
	}
}

void UBuildGridMapMapConfigWidget::OnHexTileOrientationSelectionChanged(FString SelectedItem,
                                                                        ESelectInfo::Type SelectionType)
{
	switch (SelectionType)
	{
	case ESelectInfo::OnMouseClick:
		{
			auto TileOrientation = GetTileOrientationFromString(SelectedItem);
			OnTileOrientationChanged.Broadcast(TileOrientation);
		}
		break;
	default:
		break;
	}
}

EGridMapType UBuildGridMapMapConfigWidget::GetMapTypeFromString(const FString& InString) const
{
	UEnum* MapTypeEnum = StaticEnum<EGridMapType>();
	int32 EnumCount = MapTypeEnum->NumEnums() - 1; // -1 是为了排除枚举的 _MAX 类型值（如果存在）
	EGridMapType ResultType = EGridMapType::HEX_STANDARD;
	for (int32 i = 0; i < EnumCount; ++i)
	{
		FString EnumName = MapTypeEnum->GetDisplayNameTextByIndex(i).ToString();
		if (EnumName == InString)
		{
			ResultType = static_cast<EGridMapType>(i);
			break;
		}
	}

	return ResultType;
}

EGridMapDrawMode UBuildGridMapMapConfigWidget::GetMapDrawModeFromString(const FString& InString) const
{
	UEnum* MapDrawModeEnum = StaticEnum<EGridMapDrawMode>();
	int32 EnumCount = MapDrawModeEnum->NumEnums() - 1; // -1 是为了排除枚举的 _MAX 类型值（如果存在）
	EGridMapDrawMode ResultType = EGridMapDrawMode::BaseOnRowColumn;
	for (int32 i = 0; i < EnumCount; ++i)
	{
		FString EnumName = MapDrawModeEnum->GetDisplayNameTextByIndex(i).ToString();
		if (EnumName == InString)
		{
			ResultType = static_cast<EGridMapDrawMode>(i);
			break;
		}
	}

	return ResultType;
}

ETileOrientationFlag UBuildGridMapMapConfigWidget::GetTileOrientationFromString(const FString& InString) const
{
	UEnum* HexTileOrientationEnum = StaticEnum<ETileOrientationFlag>();
	int32 EnumCount = HexTileOrientationEnum->NumEnums() - 1; // -1 是为了排除枚举的 _MAX 类型值（如果存在）
	ETileOrientationFlag ResultType = ETileOrientationFlag::FLAT;
	for (int32 i = 0; i < EnumCount; ++i)
	{
		FString EnumName = HexTileOrientationEnum->GetDisplayNameTextByIndex(i).ToString();
		if (EnumName == InString)
		{
			ResultType = static_cast<ETileOrientationFlag>(i);
			break;
		}
	}

	return ResultType;
}

void UBuildGridMapMapConfigWidget::UpdateShowLine(EGridMapType InMapType, EGridMapDrawMode InDrawMode)
{
	// 创建一个Map来存储每个组件的可见性
	TMap<UWidget*, ESlateVisibility> VisibilityMap;

	// 初始化所有组件为Collapsed
	TArray<UWidget*> AllWidgets = {
		TileOrientationComboBox,
		GridSizeXTextBox,
		GridSizeYTextBox,
		GridRadiusTextBox,
		MapRowTextBox,
		MapColumnTextBox,
		MapRadiusTextBox
	};

	for (auto* Widget : AllWidgets)
	{
		VisibilityMap.Add(Widget, ESlateVisibility::Collapsed);
	}

	// 应用可见性规则
	if (const auto* MapTypeRules = VisibilityRules.Find(InMapType))
	{
		if (const auto* VisibleWidgets = MapTypeRules->Find(InDrawMode))
		{
			for (auto* Widget : *VisibleWidgets)
			{
				if (Widget)
				{
					VisibilityMap[Widget] = ESlateVisibility::Visible;
				}
			}
		}
	}

	// 如果是体积绘制模式，记录日志
	if (InDrawMode == EGridMapDrawMode::BaseOnVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("Todo: 体积绘制功能待实现"));
	}

	// 应用可见性设置
	for (const auto& Pair : VisibilityMap)
	{
		auto Parent = Pair.Key->GetParent();
		if (Parent)
		{
			Parent->SetVisibility(Pair.Value);
		}
		else
		{
			Pair.Key->SetVisibility(Pair.Value);
		}
	}
}

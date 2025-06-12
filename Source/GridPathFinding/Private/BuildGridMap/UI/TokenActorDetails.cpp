#include "BuildGridMap/UI/TokenActorDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "DesktopPlatformModule.h"
#include "GridPathFinding.h"
#include "IDesktopPlatform.h"
#include "TokenActor.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "TokenActorDetails"

TSharedRef<IDetailCustomization> FTokenActorDetails::MakeInstance()
{
	return MakeShareable(new FTokenActorDetails);
}

void FTokenActorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// 获取选中的对象
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);

	if (SelectedObjects.Num() == 1)
	{
		SelectedTokenActor = Cast<ATokenActor>(SelectedObjects[0].Get());
	}

	// 添加自定义类别
	IDetailCategoryBuilder& TokenActorCategory = DetailBuilder.EditCategory("TokenActorTools", FText::GetEmpty(), ECategoryPriority::Important);

	// 添加导出JSON按钮
	TokenActorCategory.AddCustomRow(LOCTEXT("ExportJsonRow", "导出JSON"))
	                  .WholeRowContent()
	[
		SNew(SButton)
		.Text(LOCTEXT("ExportJson", "导出为JSON"))
		.ToolTipText(LOCTEXT("ExportJsonTooltip", "将当前TokenActor导出为JSON文件"))
		.OnClicked(this, &FTokenActorDetails::OnExportJsonClicked)
		.IsEnabled(this, &FTokenActorDetails::IsTokenActorValid)
	];

	// 添加导入JSON按钮
	TokenActorCategory.AddCustomRow(LOCTEXT("ImportJsonRow", "导入JSON"))
	                  .WholeRowContent()
	[
		SNew(SButton)
		.Text(LOCTEXT("ImportJson", "从JSON导入"))
		.ToolTipText(LOCTEXT("ImportJsonTooltip", "从JSON文件导入数据到当前TokenActor"))
		.OnClicked(this, &FTokenActorDetails::OnImportJsonClicked)
		.IsEnabled(this, &FTokenActorDetails::IsTokenActorValid)
	];
}

FReply FTokenActorDetails::OnExportJsonClicked()
{
	if (!SelectedTokenActor.IsValid())
	{
		return FReply::Handled();
	}

	// 打开文件保存对话框
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> SaveFilenames;
		FString DefaultPath = FPaths::ProjectSavedDir();
		FString DefaultFile = SelectedTokenActor->GetName() + TEXT(".json");

		bool bSaved = DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("SaveTokenActorJson", "保存TokenActor为JSON").ToString(),
			DefaultPath,
			DefaultFile,
			TEXT("JSON文件|*.json"),
			EFileDialogFlags::None,
			SaveFilenames
		);

		if (bSaved && SaveFilenames.Num() > 0)
		{
			// 序列化并保存
			FString JsonString = SelectedTokenActor->SerializeToJson();
			if (FFileHelper::SaveStringToFile(JsonString, *SaveFilenames[0]))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("TokenActorSaved", "TokenActor已成功保存到: {0}"), FText::FromString(SaveFilenames[0])));

				// 将JSON字符串复制到剪贴板
				FPlatformApplicationMisc::ClipboardCopy(*JsonString);
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveFailed", "保存失败！"));
			}
		}
	}
	UE_LOG(LogGridPathFinding, Log, TEXT("[FTokenActorDetails.OnExportJsonClicked]"));
	return FReply::Handled();
}

FReply FTokenActorDetails::OnImportJsonClicked()
{
	if (!SelectedTokenActor.IsValid())
	{
		return FReply::Handled();
	}

	// 打开文件选择对话框
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> OpenFilenames;
		FString DefaultPath = FPaths::ProjectSavedDir();

		bool bOpened = DesktopPlatform->OpenFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("LoadTokenActorJson", "加载TokenActor JSON").ToString(),
			DefaultPath,
			TEXT(""),
			TEXT("JSON文件|*.json"),
			EFileDialogFlags::None,
			OpenFilenames
		);

		if (bOpened && OpenFilenames.Num() > 0)
		{
			// 加载并反序列化
			if (SelectedTokenActor->LoadFromJsonFile(OpenFilenames[0]))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("TokenActorLoaded", "TokenActor已成功从以下文件加载: {0}"), FText::FromString(OpenFilenames[0])));

				// 通知编辑器Actor已更改
				SelectedTokenActor->MarkPackageDirty();

				// TODO：模型刷新问题
				if (GEditor)
				{
					GEditor->RedrawLevelEditingViewports(true);
					GEditor->NoteSelectionChange();
				}
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("LoadFailed", "加载失败！"));
			}
		}
	}
	UE_LOG(LogGridPathFinding, Log, TEXT("[FTokenActorDetails.OnImportJsonClicked]"));
	return FReply::Handled();
}

bool FTokenActorDetails::IsTokenActorValid() const
{
	return SelectedTokenActor.IsValid();
}

#undef LOCTEXT_NAMESPACE

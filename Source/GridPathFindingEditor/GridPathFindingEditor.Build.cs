using UnrealBuildTool;

public class GridPathFindingEditor : ModuleRules
{
    public GridPathFindingEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "GridPathFinding"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "AssetRegistry",
                "EditorStyle",
                "EditorWidgets",
                "ToolMenus"
            }
        );
    }
}
//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)
#define ICON_FONT(...) FSlateFontInfo(RootToContentDir("Fonts/FontAwesome", TEXT(".ttf")), __VA_ARGS__)
#define PLUGIN_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FDungeonArchitectStyle::InResource( RelativePath, ".png" ), __VA_ARGS__ )
#define PLUGIN_BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( FDungeonArchitectStyle::InResource( RelativePath, ".png" ), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FDungeonArchitectStyle::StyleInstance = nullptr;

FString FDungeonArchitectStyle::InResource(const FString& RelativePath, const ANSICHAR* Extension) {
    static FString ResourcesDir = IPluginManager::Get().FindPlugin(TEXT("DungeonArchitect"))->GetBaseDir() /
        TEXT("Resources");
    return (ResourcesDir / RelativePath) + Extension;
}

/*
void FDungeonArchitectStyle::SetupGeneralStyles()
{
	NormalText = FTextBlockStyle()
		.SetFont(DEFAULT_FONT("Regular", FCoreStyle::RegularTextSize))
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetShadowOffset(FVector2D::ZeroVector)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		.SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f))
		.SetHighlightShape(BOX_BRUSH("Common/TextBlockHighlightShape", FMargin(3.f / 8.f)));
}
*/

void FDungeonArchitectStyle::Initialize() {
    if (!StyleInstance.IsValid()) {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FDungeonArchitectStyle::Shutdown() {
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FDungeonArchitectStyle::GetStyleSetName() {
    static FName StyleSetName(TEXT("DungeonArchitectStyle"));
    return StyleSetName;
}

TSharedRef<class FSlateStyleSet> FDungeonArchitectStyle::Create() {
    TSharedRef<FStyle> StyleRef = MakeShareable(new FStyle);
    StyleRef->Initialize();

    return StyleRef;
}

const ISlateStyle& FDungeonArchitectStyle::Get() {
    return *StyleInstance;
}

FDungeonArchitectStyle::FStyle::FStyle()
    : FSlateStyleSet(FDungeonArchitectStyle::GetStyleSetName()) {
}

void FDungeonArchitectStyle::FStyle::Initialize() {
    const FVector2D Icon4x4(4.0f, 4.0f);
    const FVector2D Icon8x8(8.0f, 8.0f);
    const FVector2D Icon16x16(16.0f, 16.0f);
    const FVector2D Icon20x20(20.0f, 20.0f);
    const FVector2D Icon23x23(23.0f, 23.0f);
    const FVector2D Icon24x24(24.0f, 24.0f);
    const FVector2D Icon32x32(32.0f, 32.0f);
    const FVector2D Icon36x36(36.0f, 36.0f);
    const FVector2D Icon40x40(40.0f, 40.0f);
    const FVector2D Icon48x48(48.0f, 48.0f);
    const FVector2D Icon64x64(64.0f, 64.0f);
    const FVector2D Icon80x80(80.0f, 80.0f);
    const FVector2D Icon92x92(92.0f, 92.0f);
    const FVector2D Icon128x128(128.0f, 128.0f);

    SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
    SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

    NormalText = FTextBlockStyle()
                 .SetFont(DEFAULT_FONT("Regular", FCoreStyle::RegularTextSize))
                 .SetColorAndOpacity(FSlateColor::UseForeground())
                 .SetShadowOffset(FVector2D::ZeroVector)
                 .SetShadowColorAndOpacity(FLinearColor::Black)
                 .SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f))
                 .SetHighlightShape(BOX_BRUSH("Common/TextBlockHighlightShape", FMargin(3.f / 8.f)));

    // Generic styles
    {
        Set("DungeonArchitect.TabIcon", new PLUGIN_IMAGE_BRUSH("Icons/DungeonEditMode/icon_DungeonEd_40x", Icon40x40));
        Set("DungeonArchitect.TabIcon.Small", new PLUGIN_IMAGE_BRUSH("Icons/DungeonEditMode/icon_DungeonEd_40x", Icon40x40));

        Set("DungeonArchitect.OpenHelpSystem", new PLUGIN_IMAGE_BRUSH("Icons/HelpSystem/DA_UserGuide_40x", Icon40x40));
        Set("DungeonArchitect.Buttons.Flat", new PLUGIN_BOX_BRUSH("Icons/Common/FlatButton_8x", Icon8x8));
        Set("DungeonArchitect.Arrows.Generic.Right", new PLUGIN_IMAGE_BRUSH("Icons/Common/arrow_right_16x", Icon16x16));

        Set("DungeonArchitect.Icons.Play", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_Play_40x", Icon40x40));
        Set("DungeonArchitect.Icons.Stop", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_Stop_40x", Icon40x40));
    }

    // Editor Mode Styles
    {
        Set("DungeonArchitect.ModePaint",
            new PLUGIN_IMAGE_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Paint_40x", Icon40x40));
        Set("DungeonArchitect.ModeRectangle",
            new PLUGIN_IMAGE_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Rectangle_40x", Icon40x40));
        Set("DungeonArchitect.ModeBorder",
            new PLUGIN_IMAGE_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Border_40x", Icon40x40));
        Set("DungeonArchitect.ModeSelect",
            new PLUGIN_IMAGE_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Select_40x", Icon40x40));
    }

    // Toolbar
    {
        Set("DungeonArchitect.Toolbar.IconMain", new PLUGIN_IMAGE_BRUSH("Icons/Common/da_logo_48", Icon48x48));
        Set("DungeonArchitect.ToolbarItem.IconLaunchPad",
            new PLUGIN_IMAGE_BRUSH("Icons/LaunchPad/icon_tool_launchpad_16x", Icon16x16));
    }

    // Launchpad
    {
        Set("DungeonArchitect.LaunchPad.IconSideBar",
            new PLUGIN_IMAGE_BRUSH("Icons/LaunchPad/da_logo_sidebar_128", Icon128x128));
    }

    // Snap Connection Editor
    {
        Set("SnapConnectionEditor.Rebuild", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_compile_40x", Icon40x40));
    }
    
    // Snap Editor
    {
        Set("DA.SnapEd.RuleArrow", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/icon_FlowEd_RuleArrow_36x", Icon36x36));
        Set("DA.SnapEd.GraphIcon", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/icon_FlowEd_GraphIcon_32x", Icon32x32));
        Set("DA.SnapEd.NodeIcon", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/icon_FlowEd_NodeIcon_32x", Icon32x32));

        // State or conduit node
        Set("DA.SnapEd.StateNode.Body", new BOX_BRUSH("/Persona/StateMachineEditor/StateNode_Node_Body",
                                                      FMargin(16.f / 64.f, 25.f / 64.f, 16.f / 64.f, 16.f / 64.f)));
        Set("DA.SnapEd.StateNode.ColorSpill", new BOX_BRUSH("/Persona/StateMachineEditor/StateNode_Node_ColorSpill",
                                                            FMargin(4.0f / 64.0f, 4.0f / 32.0f)));

        Set("DA.SnapEd.StateNode.Icon", new IMAGE_BRUSH("/Persona/StateMachineEditor/State_Node_Icon_32x", Icon16x16));
        Set("DA.SnapEd.ConduitNode.Icon",
            new IMAGE_BRUSH("/Persona/StateMachineEditor/Conduit_Node_Icon_32x", Icon16x16));

        Set("DA.SnapEd.StateNode.Pin.BackgroundHovered", new BOX_BRUSH(
                "/Persona/StateMachineEditor/StateNode_Pin_HoverCue",
                FMargin(12.0f / 64.0f, 12.0f / 64.0f, 12.0f / 64.0f, 12.0f / 64.0f)));
        Set("DA.SnapEd.StateNode.Pin.Background", new FSlateNoResource());

        Set("DA.SnapEd.Graph.ArrowDependent",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_ArrowDependent_23x", Icon23x23));

        Set("DA.SnapEd.SuccessIcon", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/icon_FlowEd_Success_16x", Icon16x16));
        Set("DA.SnapEd.GroupBorder", new BOX_BRUSH("Common/GroupBorder", FMargin(4.0f / 16.0f)));

        Set("DA.SnapEd.DebugDoorIcon",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/flow_debug_door_16x", Icon16x16));
        Set("DA.SnapEd.DebugDoorIconIn",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/flow_debug_door_in_16x", Icon16x16));
        Set("DA.SnapEd.DebugDoorIconOut",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/flow_debug_door_out_16x", Icon16x16));

        Set("DA.SnapEd.SwitchToDesignMode",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/AppModes/FlowDesignMode_40x", Icon40x40));
        Set("DA.SnapEd.SwitchToDesignMode.Small",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/AppModes/FlowDesignMode_20x", Icon20x20));
        Set("DA.SnapEd.SwitchToVisualizeMode",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/AppModes/FlowVisualizeMode_40x", Icon40x40));
        Set("DA.SnapEd.SwitchToVisualizeMode.Small",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/AppModes/FlowVisualizeMode_20x", Icon20x20));
        Set("DA.SnapEd.SwitchToDebugMode",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/AppModes/FlowDebugMode_40x", Icon40x40));
        Set("DA.SnapEd.SwitchToDebugMode.Small",
            new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/AppModes/FlowDebugMode_20x", Icon20x20));

        // Buttons
        FButtonStyle Button = FButtonStyle()
                              .SetNormalPadding(FMargin(2, 2, 2, 2))
                              .SetPressedPadding(FMargin(2, 3, 2, 1));

        Set("DA.SnapEd.Buttons.LinkNonDependent", FButtonStyle(Button)
                                               .SetNormal(PLUGIN_IMAGE_BRUSH(
                                                   "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_LinkNonDependent_OFF_24x",
                                                   Icon24x24))
                                               .SetHovered(PLUGIN_IMAGE_BRUSH(
                                                   "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_LinkNonDependent_OFF_24x",
                                                   Icon24x24))
                                               .SetPressed(PLUGIN_IMAGE_BRUSH(
                                                   "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_LinkNonDependent_24x",
                                                   Icon24x24)));

        Set("DA.SnapEd.Buttons.LinkDependent", FButtonStyle(Button)
                                            .SetNormal(PLUGIN_IMAGE_BRUSH(
                                                "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_LinkDependent_OFF_24x",
                                                Icon24x24))
                                            .SetHovered(PLUGIN_IMAGE_BRUSH(
                                                "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_LinkDependent_OFF_24x",
                                                Icon24x24))
                                            .SetPressed(PLUGIN_IMAGE_BRUSH(
                                                "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_LinkDependent_24x",
                                                Icon24x24)));

        Set("DA.SnapEd.Buttons.Rewind", FButtonStyle(Button)
                                     .SetNormal(PLUGIN_IMAGE_BRUSH(
                                         "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Rewind_OFF_24x", Icon24x24))
                                     .SetHovered(PLUGIN_IMAGE_BRUSH(
                                         "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Rewind_OFF_24x", Icon24x24))
                                     .SetPressed(PLUGIN_IMAGE_BRUSH(
                                         "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Rewind_24x", Icon24x24)));

        Set("DA.SnapEd.Buttons.Step", FButtonStyle(Button)
                                   .SetNormal(PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Step_OFF_24x",
                                                                 Icon24x24))
                                   .SetHovered(PLUGIN_IMAGE_BRUSH(
                                       "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Step_OFF_24x", Icon24x24))
                                   .SetPressed(PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Step_24x",
                                                                  Icon24x24)));


        Set("DA.SnapEd.Buttons.Play", FButtonStyle(Button)
                                   .SetNormal(PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Play_OFF_24x",
                                                                 Icon24x24))
                                   .SetHovered(PLUGIN_IMAGE_BRUSH(
                                       "Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Play_OFF_24x", Icon24x24))
                                   .SetPressed(PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/GraphIcons/icon_FlowEd_Play_24x",
                                                                  Icon24x24)));


        // Command icons
        Set("SnapEd.ValidateGrammarGraph", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_validate_40x", Icon40x40));
        Set("SnapEd.ValidateGrammarGraph.Small", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_validate_40x", Icon20x20));
        Set("SnapEd.BuildGraph", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_compile_40x", Icon40x40));
        Set("SnapEd.BuildGraph.Small", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_compile_40x", Icon20x20));
        Set("SnapEd.Performance", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_stats_40x", Icon40x40));
        Set("SnapEd.Performance.Small", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_stats_40x", Icon20x20));
        Set("SnapEd.Settings", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_game_settings_40x", Icon40x40));
        Set("SnapEd.Settings.Small", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_game_settings_40x", Icon20x20));

        Set("SnapEd.DebugStepForward", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/icon_step_40x", Icon40x40));
        Set("SnapEd.DebugStepForward.Small", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/icon_step_40x", Icon20x20));
        Set("SnapEd.DebugRestart", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/icon_restart_40x", Icon40x40));
        Set("SnapEd.DebugRestart.Small", new PLUGIN_IMAGE_BRUSH("Icons/Editors/SnapEd/DebugIcons/icon_restart_40x", Icon20x20));
    }


    // Grid Flow Editor
    {
        Set("FlowEditor.Build", new IMAGE_BRUSH("Icons/icon_compile_40x", Icon40x40));
        Set("FlowEditor.Build.Small", new IMAGE_BRUSH("Icons/icon_compile_40x", Icon20x20));
        Set("FlowEditor.Performance", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_stats_40x", Icon40x40));
        Set("FlowEditor.Settings", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_game_settings_40x", Icon40x40));
        Set("FlowEditor.ShowEditorSettings", new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_settings_16x", Icon16x16));
        Set("FlowEditor.ShowPreviewDungeonSettings",
            new PLUGIN_IMAGE_BRUSH("Icons/Common/icon_settings_16x", Icon16x16));

        FMargin ExecNodeBodyMargin(4.0f / 64.0f, 4.0f / 32.0f);
        Set("FlowEditor.ExecNode.Body.Default", new PLUGIN_BOX_BRUSH("Icons/Editors/FlowEd/Nodes/ExecNodes/gridflow_node_default_64x", ExecNodeBodyMargin));
        Set("FlowEditor.ExecNode.Body.Green", new PLUGIN_BOX_BRUSH("Icons/Editors/FlowEd/Nodes/ExecNodes/gridflow_node_green_64x", ExecNodeBodyMargin));
        Set("FlowEditor.ExecNode.Body.Orange", new PLUGIN_BOX_BRUSH("Icons/Editors/FlowEd/Nodes/ExecNodes/gridflow_node_orange_64x", ExecNodeBodyMargin));
        Set("FlowEditor.ExecNode.Body.Red", new PLUGIN_BOX_BRUSH("Icons/Editors/FlowEd/Nodes/ExecNodes/gridflow_node_red_64x", ExecNodeBodyMargin));

        Set("FlowEditor.AbstractNode.Body", new PLUGIN_IMAGE_BRUSH("Icons/Editors/FlowEd/Nodes/AbstractNode/abstract_graph_node_64x", Icon64x64));
        Set("FlowEditor.AbstractNode.Body.Shadow", new PLUGIN_IMAGE_BRUSH("Icons/Editors/FlowEd/Nodes/AbstractNode/abstract_graph_node_shadow_92x", Icon92x92));
        Set("FlowEditor.AbstractNode.Body.Selected", new PLUGIN_IMAGE_BRUSH("Icons/Editors/FlowEd/Nodes/AbstractNode/abstract_graph_node_selected_92x", Icon92x92));

        Set("FlowEditor.ItemWidget.Body", new PLUGIN_BOX_BRUSH("Icons/Editors/FlowEd/Nodes/Items/GridFlowItemBody",
                                                                   Icon20x20,
                                                                   FMargin(9.0f / 20.0f, 1.0f / 20.0f, 9.0f / 20.0f,
                                                                       3.0f / 20.0f)));
        Set("FlowEditor.ItemWidget.Body.Color", FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
        Set("FlowEditor.ItemWidget.Body.HoveredColor", FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
        Set("FlowEditor.ItemWidget.Body.Selected", new PLUGIN_BOX_BRUSH( "Icons/Editors/FlowEd/Nodes/Items/GridFlowItemSelection", Icon20x20, FMargin(9.0f / 20.0f, 1.0f / 20.0f, 9.0f / 20.0f, 3.0f / 20.0f)));

        Set("FlowEditor.TestRunner.BarBody", new BOX_BRUSH("Common/Scrollbar_Thumb", FMargin(4.f / 16.f)));

        Set( "FlowEditor.ExecGraph.EditorBranding", FTextBlockStyle(NormalText)
            .SetFont( DEFAULT_FONT( "BoldCondensed", 32 ) )
            .SetColorAndOpacity( FLinearColor(0.8, 0.8f, 0.8f, 0.1f) )
            .SetShadowOffset( FVector2D::ZeroVector )
        );
        
        FTextBlockStyle GraphNodeTitle = FTextBlockStyle(NormalText)
            .SetFont(DEFAULT_FONT("Bold", 8));
        Set("FlowEditor.ItemWidget.Text", GraphNodeTitle);


        Set("PerfBackground", new BOX_BRUSH("Common/DarkGroupBorder", FMargin(4.0f / 16.0f)));

        const FTextBlockStyle PerfToolbarText = FTextBlockStyle()
                                                .SetFont(DEFAULT_FONT("Regular", 9))
                                                .SetColorAndOpacity(FLinearColor::White)
                                                .SetShadowOffset(FVector2D(1, 1))
                                                .SetShadowColorAndOpacity(FLinearColor::Black)
                                                .SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f))
                                                .SetHighlightShape(
                                                    BOX_BRUSH("Common/TextBlockHighlightShape", FMargin(3.f / 8.f)));
        Set("FlowEditor.Perf.ToolBar.Text", PerfToolbarText);
    }

    Set("DungeonArchitect.RoundDarkBorder", new BOX_BRUSH("Common/GroupBorderLight", FMargin(4.0f / 16.0f)));
    Set("DungeonArchitect.ListView.LargeFont", DEFAULT_FONT("Regular", 16));

    Set("DungeonArchitect.Fonts.Normal", DEFAULT_FONT("Regular", 12));

    // Close / Delete button style
    {
        const float NormalTint = 0.5f;
        const float HoverTint = 0.8f;
        const float PressedTint = 1.0f;
        FVector2D Icon12x12(12, 12);
        FString Icon = "Icons/Cross_12x";
        FButtonStyle ButtonStyle = FButtonStyle()
                                   .SetNormal(IMAGE_BRUSH(Icon, Icon12x12, FLinearColor(1, 0, 0, NormalTint)))
                                   .SetHovered(IMAGE_BRUSH(Icon, Icon12x12, FLinearColor(1, 0, 0, HoverTint)))
                                   .SetPressed(IMAGE_BRUSH(Icon, Icon12x12, FLinearColor(1, 0, 0, PressedTint)))
                                   .SetNormalPadding(FMargin(2, 2, 2, 2))
                                   .SetPressedPadding(FMargin(2, 3, 2, 1));
        Set("DungeonArchitect.Button.Close", ButtonStyle);

        Set("DungeonArchitect.Button.Close.White", FButtonStyle(ButtonStyle)
                                                   .SetNormal(IMAGE_BRUSH(
                                                       Icon, Icon12x12, FLinearColor(1, 1, 1, NormalTint)))
                                                   .SetHovered(
                                                       IMAGE_BRUSH(Icon, Icon12x12, FLinearColor(1, 1, 1, HoverTint)))
                                                   .SetPressed(IMAGE_BRUSH(
                                                       Icon, Icon12x12, FLinearColor(1, 1, 1, PressedTint)))
        );
    }

    // Add New Item button style
    {
        const float NormalTint = 0.5f;
        const float HoverTint = 0.8f;
        const float PressedTint = 1.0f;
        FLinearColor NormalColor(0.063f, 0.49f, 0.675f, NormalTint);
        FLinearColor HoveredColor(0.063f, 0.49f, 0.675f, HoverTint);
        FLinearColor PressedColor(0.063f, 0.49f, 0.675f, PressedTint);

        FMargin Margin(4.0f / 16.0f);
        FButtonStyle Button = FButtonStyle()
                              .SetNormal(BOX_BRUSH("Common/Button", FVector2D(32, 32), 8.0f / 32.0f, NormalColor))
                              .SetHovered(BOX_BRUSH("Common/Button_Hovered", FVector2D(32, 32), 8.0f / 32.0f,
                                                    HoveredColor))
                              .SetPressed(BOX_BRUSH("Common/Button_Pressed", FVector2D(32, 32), 8.0f / 32.0f,
                                                    PressedColor))
                              .SetNormalPadding(FMargin(2, 2, 2, 2))
                              .SetPressedPadding(FMargin(2, 3, 2, 1));
        Set("DungeonArchitect.Button.Blue", Button);

        Set("DungeonArchitect.FlatButton.Blue", FButtonStyle(Button)
                                                .SetNormal(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, NormalColor))
                                                .SetHovered(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, HoveredColor))
                                                .SetPressed(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, PressedColor))
        );
    }
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef DEFAULT_FONT
#undef ICON_FONT
#undef PLUGIN_IMAGE_BRUSH
#undef PLUGIN_BOX_BRUSH


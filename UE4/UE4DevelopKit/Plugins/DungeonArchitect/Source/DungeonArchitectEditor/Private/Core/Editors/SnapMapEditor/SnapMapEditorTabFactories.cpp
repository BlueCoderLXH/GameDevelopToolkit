//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/SnapMapEditorTabFactories.h"

#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorTabs.h"

#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "DungeonFlowEditorTabFactories"

DEFINE_FLOW_TAB_FACTORY(GrammarEditor,
                        LOCTEXT("Caption_GrammarEditor", "Mission Grammar"),
                        "LevelEditor.Tabs.Viewports",
                        LOCTEXT("TooltipMenu_GrammarEditor", "Show the Mission Graph editor"),
                        LOCTEXT("TooltipTab_GrammarEditor", "Design you mission graph in this editor"))


DEFINE_FLOW_TAB_FACTORY(ExecutionGraph,
                        LOCTEXT("Caption_ExecutionGraph", "Execution Graph"),
                        "LevelEditor.Tabs.Viewports",
                        LOCTEXT("TooltipMenu_ExecutionGraph", "Show the Execution Graph editor"),
                        LOCTEXT("TooltipTab_ExecutionGraph",
                            "Control how the rules will be executed using this execution graph"))


DEFINE_FLOW_TAB_FACTORY(ResultGraph,
                        LOCTEXT("Caption_ResultGraph", "Result Graph"),
                        "LevelEditor.Tabs.Viewports",
                        LOCTEXT("TooltipMenu_ResultGraph", "Show the Result Graph editor"),
                        LOCTEXT("TooltipTab_ResultGraph", "Shows the executed result graph"))


DEFINE_FLOW_TAB_FACTORY(Details,
                        LOCTEXT("Caption_Detail", "Details"),
                        "LevelEditor.Tabs.Details",
                        LOCTEXT("TooltipMenu_Detail", "Show the Details panel"),
                        LOCTEXT("TooltipTab_Detail", "Edit properties using the details panel"))

DEFINE_FLOW_TAB_FACTORY(ErrorList,
                        LOCTEXT("Caption_ErrorList", "ErrorList"),
                        "LevelEditor.Tabs.Details",
                        LOCTEXT("TooltipMenu_ErrorList", "Show the Error List panel"),
                        LOCTEXT("TooltipTab_ErrorList",
                            "Displays all the possible errors with your graph grammar.  Double click to highlight the error"
                        ))


DEFINE_FLOW_TAB_FACTORY(Viewport,
                        LOCTEXT("Caption_Viewport", "Viewport"),
                        "LevelEditor.Tabs.Details",
                        LOCTEXT("TooltipMenu_Viewport", "Show the 3D viewport"),
                        LOCTEXT("TooltipTab_Viewport", "Visualize and debug the dungeon from this 3D viewport"))


DEFINE_FLOW_TAB_FACTORY(ViewportSceneSettings,
                        LOCTEXT("Caption_ViewportSceneSettings", "Scene Settings"),
                        "LevelEditor.Tabs.Details",
                        LOCTEXT("TooltipMenu_ViewportSceneSettings", "Show the 3D viewport scene settings"),
                        LOCTEXT("TooltipTab_ViewportSceneSettings", "Modify the scene settings of the 3D viewport"))


DEFINE_FLOW_TAB_FACTORY(VisualizeResultGraph,
                        LOCTEXT("Caption_VisualizeResultGraph", "Result Graph"),
                        "LevelEditor.Tabs.Viewports",
                        LOCTEXT("TooltipMenu_VisualizeResultGraph", "Show the Result Graph editor"),
                        LOCTEXT("TooltipTab_VisualizeResultGraph", "Shows the executed result graph"))


DEFINE_FLOW_TAB_FACTORY(VisualizeDetails,
                        LOCTEXT("Caption_VisualizeDetail", "Details"),
                        "LevelEditor.Tabs.Details",
                        LOCTEXT("TooltipMenu_VisualizeDetail", "Show the Details panel"),
                        LOCTEXT("TooltipTab_VisualizeDetail", "Build the dungeon using the details panel"))

DEFINE_FLOW_TAB_FACTORY_CUSTOM_WIDGET(Performance,
                        LOCTEXT("Caption_Performance", "Performance"),
                        "LevelEditor.Tabs.Performance",
                        LOCTEXT("TooltipMenu_Detail", "Run the graph thousands of times and measure the performance"),
                        LOCTEXT("TooltipTab_Detail", "Run the graph thousands of times and measure the performance"))




#undef LOCTEXT_NAMESPACE


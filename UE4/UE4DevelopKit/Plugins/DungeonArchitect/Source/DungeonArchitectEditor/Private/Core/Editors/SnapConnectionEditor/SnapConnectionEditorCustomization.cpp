//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditorCustomization.h"

#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

///////////////// FSnapMapConnectionActorCustomization ////////////////

void FSnapConnectionActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    ASnapConnectionActor* ConnectionActor = FDungeonEditorUtils::GetBuilderObject<ASnapConnectionActor>(
        &DetailBuilder);
    if (!ConnectionActor) {
        return;
    }

    if (ConnectionActor->ConnectionComponent) {
        IDetailCategoryBuilder& ConfigCategory = DetailBuilder.EditCategory("SnapMap");
        TArray<UObject*> Configs;
        Configs.Add(ConnectionActor->ConnectionComponent);
        ConfigCategory.AddExternalObjectProperty(Configs, GET_MEMBER_NAME_CHECKED(USnapConnectionComponent, ConnectionInfo));
        ConfigCategory.AddExternalObjectProperty(Configs, GET_MEMBER_NAME_CHECKED(USnapConnectionComponent, ConnectionConstraint));
    }

}

TSharedRef<IDetailCustomization> FSnapConnectionActorCustomization::MakeInstance() {
    return MakeShareable(new FSnapConnectionActorCustomization);
}


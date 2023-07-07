//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Layout/GraphLayout.h"

struct FVerletGraphLayoutConfig {
    int32 NumIterations = 100;
    int32 NumConstraintIterations = 3;
    FVector2D Gravity = FVector2D(2, 0);

    float NodeRadius = 60;
    float LinkRestingDistance = 120;
    float LinkStrength = 1.0f;
    float KeyLockStrength = 0.5f;
};

struct FLayoutVerletNode;
struct FLayoutVerletLink;

class DUNGEONARCHITECTEDITOR_API FVerletGraphLayout : public IGraphLayout {
public:
    FVerletGraphLayout(const FVerletGraphLayoutConfig& InConfig) : Config(InConfig) {
    }

    virtual ~FVerletGraphLayout() {
    }

    virtual void PerformLayout(UEdGraph_Grammar* Graph) override;

private:
    void SolveVerlet();
    void SolveVerletConstraint(TSharedPtr<FLayoutVerletNode> A, TSharedPtr<FLayoutVerletNode> B, float RestingDistance,
                               float Strength);

private:
    FVerletGraphLayoutConfig Config;
    TArray<TSharedPtr<FLayoutVerletNode>> VerletNodes;
    TArray<TSharedPtr<FLayoutVerletLink>> VerletLinks;
    FDateTime StartTime;
};


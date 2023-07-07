//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/MathUtils.h"

#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

TArray<int32> FMathUtils::GetShuffledIndices(int32 Count, const FRandomStream& Random) {
    TArray<int32> Indices;
    for (int i = 0; i < Count; i++) {
        Indices.Add(i);
    }

    Shuffle(Indices, Random);

    return Indices;
}

void BlurUtils::boxBlurH_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r) {
    float iarr = 1.0f / (r + r + 1);
    for (int32 i = 0; i < h; i++) {
        int32 ti = i * w, li = ti, ri = ti + r;
        float fv = scl[ti], lv = scl[ti + w - 1], val = (r + 1) * fv, weight;
        for (int32 j = 0; j < r; j++) val += scl[ti + j];
        for (int32 j = 0; j <= r; j++) {
            val += scl[ri++] - fv;
            weight = weights[ti];
            tcl[ti] = scl[ti] * weight + BlurRound(val * iarr) * (1 - weight);
            ti++;
        }

        for (int32 j = r + 1; j < w - r; j++) {
            val += scl[ri++] - scl[li++];
            weight = weights[ti];
            tcl[ti] = scl[ti] * weight + BlurRound(val * iarr) * (1 - weight);
            ti++;
        }

        for (int32 j = w - r; j < w; j++) {
            val += lv - scl[li++];
            weight = weights[ti];
            tcl[ti] = scl[ti] * weight + BlurRound(val * iarr) * (1 - weight);
            ti++;
        }
    }
}

void BlurUtils::boxBlurT_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r) {
    float iarr = 1.0f / (r + r + 1);
    for (int32 i = 0; i < w; i++) {
        int32 ti = i, li = ti, ri = ti + r * w;
        float fv = scl[ti], lv = scl[ti + w * (h - 1)], val = (r + 1) * fv, weight;
        for (int32 j = 0; j < r; j++) val += scl[ti + j * w];
        for (int32 j = 0; j <= r; j++) {
            val += scl[ri] - fv;
            weight = weights[ti];
            tcl[ti] = scl[ti] * weight + BlurRound(val * iarr) * (1 - weight);
            ri += w;
            ti += w;
        }

        for (int32 j = r + 1; j < h - r; j++) {
            val += scl[ri] - scl[li];
            weight = weights[ti];
            tcl[ti] = scl[ti] * weight + BlurRound(val * iarr) * (1 - weight);
            li += w;
            ri += w;
            ti += w;
        }

        for (int32 j = h - r; j < h; j++) {
            val += lv - scl[li];
            weight = weights[ti];
            tcl[ti] = scl[ti] * weight + BlurRound(val * iarr) * (1 - weight);
            li += w;
            ti += w;
        }
    }
}

void BlurUtils::boxBlur_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r) {
    int32 Count = w * h;
    for (int32 i = 0; i < Count; i++) tcl[i] = scl[i];
    boxBlurH_4(tcl, scl, weights, w, h, r);
    boxBlurT_4(scl, tcl, weights, w, h, r);
}

TArray<int32> BlurUtils::boxesForGauss(float sigma, float n) // standard deviation, number of boxes
{
    float wIdeal = FMath::Sqrt((12 * sigma * sigma / n) + 1); // Ideal averaging filter width 
    int32 wl = FMath::FloorToInt(wIdeal);
    if (wl % 2 == 0) wl--;
    int32 wu = wl + 2;

    float mIdeal = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / static_cast<float>(-4 * wl - 4);
    int32 m = FMath::RoundToInt(mIdeal);
    // var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

    TArray<int32> sizes;
    for (int32 i = 0; i < n; i++) {
        sizes.Add(i < m ? wl : wu);
    }
    return sizes;
}

void BlurUtils::gaussBlur_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r) {
    TArray<int32> bxs = boxesForGauss(r, 3);
    boxBlur_4(scl, tcl, weights, w, h, (bxs[0] - 1) / 2);
    boxBlur_4(tcl, scl, weights, w, h, (bxs[1] - 1) / 2);
    boxBlur_4(scl, tcl, weights, w, h, (bxs[2] - 1) / 2);
}

FLinearColor FColorUtils::BrightenColor(const FLinearColor& InColor, float SaturationMultiplier,
                                        float BrightnessMultiplier) {
    FLinearColor HSV = InColor.LinearRGBToHSV();
    HSV.G *= SaturationMultiplier;
    HSV.B = FMath::Clamp(HSV.B * BrightnessMultiplier, 0.0f, 1.0f);
    return HSV.HSVToLinearRGB();
}


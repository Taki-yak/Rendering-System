#include "EditorLayout.h"
#include "imgui.h"

float leftX =
10.0f;

float topY =
62.0f;

float leftWidth =
270.0f;

float rightX =
1260.0f;

float rightWidth =
330.0f;

float centerX =
290.0f;

float centerWidth =
950.0f;

float bottomY =
650.0f;

float bottomHeight =
230.0f;

float layoutGap =
10.0f;

float hierarchyHeight =
560.0f;

float inspectorHeight =
430.0f;

float secondaryRightHeight =
220.0f;

float assetBrowserWidth =
1150.0f;

float ClampLayoutFloat(
    float value,
    float minValue,
    float maxValue
)
{
    if (value < minValue)
        return minValue;

    if (value > maxValue)
        return maxValue;

    return value;
}

void UpdateResponsiveEditorLayout()
{
    ImVec2 displaySize =
        ImGui::GetIO().DisplaySize;

    float screenWidth =
        displaySize.x;

    float screenHeight =
        displaySize.y;

    if (screenWidth < 800.0f)
        screenWidth = 1600.0f;

    if (screenHeight < 500.0f)
        screenHeight = 900.0f;

    float margin =
        10.0f;

    layoutGap =
        10.0f;

    // ================= TOP =================

    topY =
        62.0f;

    // ================= LEFT PANEL =================

    leftX =
        margin;

    leftWidth =
        ClampLayoutFloat(
            screenWidth * 0.17f,
            250.0f,
            320.0f
        );

    // ================= RIGHT PANEL =================

    rightWidth =
        ClampLayoutFloat(
            screenWidth * 0.20f,
            320.0f,
            400.0f
        );

    rightX =
        screenWidth -
        rightWidth -
        margin;

    // ================= CENTER VIEWPORT =================

    centerX =
        leftX +
        leftWidth +
        layoutGap;

    centerWidth =
        rightX -
        centerX -
        layoutGap;

    if (centerWidth < 400.0f)
    {
        centerWidth =
            400.0f;
    }

    // ================= BOTTOM PANEL =================

    bottomHeight =
        ClampLayoutFloat(
            screenHeight * 0.25f,
            210.0f,
            280.0f
        );

    bottomY =
        screenHeight -
        bottomHeight -
        margin;

    // ================= HIERARCHY =================

    hierarchyHeight =
        bottomY -
        topY -
        layoutGap;

    if (hierarchyHeight < 300.0f)
    {
        hierarchyHeight =
            300.0f;
    }

    // ================= RIGHT SIDE =================

    float availableRightHeight =
        bottomY -
        topY -
        layoutGap;

    inspectorHeight =
        availableRightHeight * 0.62f;

    secondaryRightHeight =
        availableRightHeight -
        inspectorHeight -
        layoutGap;

    // ================= ASSET BROWSER =================

    assetBrowserWidth =
        rightX -
        leftX -
        layoutGap;

    if (assetBrowserWidth < 650.0f)
    {
        assetBrowserWidth =
            650.0f;
    }
}
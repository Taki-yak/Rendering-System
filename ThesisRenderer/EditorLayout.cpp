#include "EditorLayout.h"
#include "imgui.h"

float leftX =
10.0f;

float topY =
70.0f;

float leftWidth =
260.0f;

float rightX =
1240.0f;

float rightWidth =
340.0f;

float centerX =
280.0f;

float centerWidth =
940.0f;

float bottomY =
650.0f;

float bottomHeight =
220.0f;

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
        screenWidth =
        1600.0f;

    if (screenHeight < 500.0f)
        screenHeight =
        900.0f;

    float margin =
        10.0f;

    float gap =
        10.0f;

    leftX =
        margin;

    topY =
        55.0f;

    leftWidth =
        ClampLayoutFloat(
            screenWidth * 0.16f,
            240.0f,
            310.0f
        );

    rightWidth =
        ClampLayoutFloat(
            screenWidth * 0.21f,
            320.0f,
            430.0f
        );

    rightX =
        screenWidth -
        rightWidth -
        margin;

    centerX =
        leftX +
        leftWidth +
        gap;

    centerWidth =
        rightX -
        centerX -
        gap;

    if (centerWidth < 420.0f)
    {
        centerWidth =
            420.0f;
    }

    bottomHeight =
        ClampLayoutFloat(
            screenHeight * 0.24f,
            190.0f,
            260.0f
        );

    bottomY =
        screenHeight -
        bottomHeight -
        margin;
}
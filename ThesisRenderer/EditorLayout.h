#pragma once

extern float leftX;
extern float topY;
extern float leftWidth;

extern float rightX;
extern float rightWidth;

extern float centerX;
extern float centerWidth;

extern float bottomY;
extern float bottomHeight;

extern float layoutGap;

extern float hierarchyHeight;

extern float inspectorHeight;

extern float secondaryRightHeight;

extern float assetBrowserWidth;

float ClampLayoutFloat(
    float value,
    float minValue,
    float maxValue
);

void UpdateResponsiveEditorLayout();
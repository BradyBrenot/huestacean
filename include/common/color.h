#pragma once

struct HsluvColor
{
	float h;
	float s;
	float l;

	HsluvColor(RgbColor from);
	HsluvColor(XyzColor from);
};

struct RgbColor
{
	float r;
	float g;
	float b;

	HsluvColor(HsluvColor from);
	HsluvColor(XyzColor from);
};

struct XyzColor
{
	float r;
	float g;
	float b;

	HsluvColor(RgbColor from);
	HsluvColor(HsluvColor from);
};
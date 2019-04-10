#pragma once

struct RgbColor;
struct HsluvColor;
struct XyzColor;

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

	RgbColor(HsluvColor from);
	RgbColor(XyzColor from);
};

struct XyzColor
{
	float r;
	float g;
	float b;

	XyzColor(RgbColor from);
	XyzColor(HsluvColor from);
};
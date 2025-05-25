#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <Windows.h>

class GFXGui
{
public:
	GFXGui();

	void Initialize(HWND hwnd);
	void BeginRender();
	void EndRender();

	void EditorStyle();
};


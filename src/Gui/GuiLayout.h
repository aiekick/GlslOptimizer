/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <ConfigAbstract.h>

#include <imgui.h>

#define OPTIMIZER_PANE "Optimizer"
#define SOURCE_PANE "Source"
#define TARGET_PANE "Target"

enum PaneFlags
{
	PANE_NONE = 0,
	PANE_OPTIMIZER = (1 << 1),
	PANE_SOURCE = (1 << 2),
	PANE_TARGET = (1 << 3),
	PANE_ALLS = PANE_OPTIMIZER | PANE_SOURCE | PANE_TARGET // | PANE_GLYPH
};

class ProjectFile;
class GuiLayout : public conf::ConfigAbstract
{
private:
	ImGuiID m_DockSpaceID = 0;
	bool m_FirstLayout = false;
	bool m_FirstStart = true;

public:
	static PaneFlags m_Pane_Shown;
	PaneFlags m_Pane_Hovered = PANE_NONE;
	PaneFlags m_Pane_LastHovered = PANE_NONE;

public:
	void StartDockPane();
	void ApplyInitialDockingLayout(ImVec2 vSize);
	void Init();
	void InitAfterFirstDisplay(ImVec2 vSize);
	void DisplayMenu(ImVec2 vSize);
	int DisplayPanes(ProjectFile *vProjectFile, int vWidgetId);
	void ShowAndFocusPane(PaneFlags vPane);
	bool IsPaneActive(PaneFlags vPane);

private:
	void ActivePane(const char *vlabel);
	bool IsPaneActive(const char *vlabel);

public: // configuration
	std::string getXml(const std::string& vOffset);
	void setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent);

public: // singleton
	static GuiLayout *Instance()
	{
		static auto *_instance = new GuiLayout();
		return _instance;
	}

protected:
	GuiLayout(); // Prevent construction
	GuiLayout(const GuiLayout&) {}; // Prevent construction by copying
	GuiLayout& operator =(const GuiLayout&) { return *this; }; // Prevent assignment
	~GuiLayout(); // Prevent unwanted destruction
};


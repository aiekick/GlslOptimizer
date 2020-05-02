﻿// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#define GLSLOPTIMIZER_VERSION "Beta 0.1"

#include "MainFrame.h"

#include <cTools.h>
#include <FileHelper.h>
#include <cctype>
#include "GLFW/glfw3.h"

#include "ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"
#include "Gui/ImGuiWidgets.h"
#include "Gui/GuiLayout.h"
#include "Helper/ImGuiThemeHelper.h"
#include "Helper/Messaging.h"
#include "Project/ProjectFile.h"
#include "Panes/OptimizerPane.h"
#include "Res/CustomFont.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#define WIDGET_ID_MAGIC_NUMBER 4577
static int widgetId = WIDGET_ID_MAGIC_NUMBER;

MainFrame::MainFrame(GLFWwindow *vWin)
{
	m_Window = vWin;
}

MainFrame::~MainFrame()
{

}

void MainFrame::Init()
{
	char buf[256];
	snprintf(buf, 255, "GlslOptimizer %s", GLSLOPTIMIZER_VERSION);
	glfwSetWindowTitle(m_Window, buf);

	ImGuiThemeHelper::Instance()->ApplyStyleColorsDefault();
	
	LoadConfigFile("config.xml");

#ifdef _DEBUG
	//std::string projectPath = PROJECT_PATH;
	//LoadProject(projectPath + "\\projects\\GlslOptimizer.glo"); // directly open this project file
#endif

	GuiLayout::Instance()->Init();
}

void MainFrame::Unit()
{
	SaveConfigFile("config.xml");
}

void MainFrame::NewProject(const std::string& vFilePathName)
{
	m_ProjectFile.New(vFilePathName);
	SetAppTitle(vFilePathName);
}

void MainFrame::LoadProject(const std::string& vFilePathName)
{
	if (m_ProjectFile.LoadAs(vFilePathName))
	{
		SetAppTitle(vFilePathName);
		
		m_ProjectFile.SetProjectChange(false);
	}
}

void MainFrame::SaveProject()
{
	m_ProjectFile.Save();
}

void MainFrame::SaveAsProject(const std::string& vFilePathName)
{
	m_ProjectFile.SaveAs(vFilePathName);

	if (m_NeedToCloseApp)
	{
		glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
	}
}

ProjectFile* MainFrame::GetProject()
{
	return &m_ProjectFile;
}

//////////////////////////////////////////////////////////////////////////////
//// DRAW ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define NEW_ID ++widgetId

void MainFrame::Display(ImVec2 vSize)
{
	widgetId = WIDGET_ID_MAGIC_NUMBER; // important for event catching on imgui widgets

	m_DisplaySize = vSize;

	DrawDockPane(vSize);

	widgetId = GuiLayout::Instance()->DisplayPanes(&m_ProjectFile, widgetId);
	
	DisplayDialogsAndPopups();

	GuiLayout::Instance()->InitAfterFirstDisplay(vSize);

	if (ImGui::BeginMainStatusBar())
	{
		Messaging::Instance()->Draw(&m_ProjectFile);

		ImGui::EndMainStatusBar();
	}
}

void MainFrame::DrawDockPane(ImVec2 vSize)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MainDockPane", (bool*)0,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking);
	ImGui::PopStyleVar();

	float barH = 0.0f;
	auto curWin = ImGui::GetCurrentWindowRead();
	if (curWin)
		barH = curWin->MenuBarHeight();
	ImGui::SetWindowSize(vSize - ImVec2(0.0f, barH));
	ImGui::SetWindowPos(ImVec2(0, 0));

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(ICON_IGFS_PROJECT " Project"))
		{
			if (ImGui::MenuItem(ICON_IGFS_FILE " New"))
			{
				m_ProjectFile.New();
			}

			if (ImGui::MenuItem(ICON_IGFS_FOLDER_OPEN " Open"))
			{
				// if change maybe save before open other
				if (m_ProjectFile.IsLoaded() && m_ProjectFile.IsThereAnyNotSavedChanged())
				{
					m_SaveDialogIfRequired = true;
					m_SaveChangeDialogActions.push_front([this]()
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("OpenProjectDlg", "Open Project File", ".glo\0\0", ".");
						m_SaveDialogIfRequired = false;
					});
				}
				else
				{
					igfd::ImGuiFileDialog::Instance()->OpenModal("OpenProjectDlg", "Open Project File", ".glo\0\0", ".");
				}
			}

			if (m_ProjectFile.IsLoaded())
			{
				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_SAVE " Save"))
				{
					if (!m_ProjectFile.Save())
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", ".glo\0\0", ".");
					}
				}

				if (ImGui::MenuItem(ICON_IGFS_SAVE " Save As"))
				{
					igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", ".glo\0\0", ".");
				}

				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_DESTROY " Close"))
				{
					// if change maybe save before close
					if (m_ProjectFile.IsThereAnyNotSavedChanged())
					{
						m_SaveDialogIfRequired = true;
						m_SaveChangeDialogActions.push_front([this]()
						{
							m_ProjectFile.Clear();
							m_SaveDialogIfRequired = false;
						});
					}
					else
					{
						m_ProjectFile.Clear();
					}
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem(ICON_IGFS_ABOUT " About"))
			{
				m_ShowAboutDialog = true;
			}
			
			ImGui::EndMenu();
		}
		
		ImGui::Spacing();

		GuiLayout::Instance()->DisplayMenu(vSize);

		ImGui::Spacing();
		
		if (ImGui::BeginMenu(ICON_IGFS_SETTINGS " Settings"))
		{
			/*if (ImGui::MenuItem("Settings"))
			{
				SettingsDlg::Instance()->OpenDialog();
			}*/

			if (ImGui::BeginMenu(ICON_IGFS_EDIT " Styles"))
			{
				ImGuiThemeHelper::Instance()->DrawMenu();

				ImGui::Separator();

				ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
				ImGui::MenuItem("Show ImGui Style", "", &m_ShowImGuiStyle);

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (m_ProjectFile.IsThereAnyNotSavedChanged())
		{
			ImGui::Spacing(200.0f);

			if (ImGui::MenuItem(ICON_IGFS_SAVE " Save"))
			{
				if (!m_ProjectFile.Save())
				{
					igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", ".glo\0\0", ".", 0);
				}
			}
		}

		ImGui::EndMenuBar();
	}

	GuiLayout::Instance()->StartDockPane();

	ImGui::End();
}

void MainFrame::DisplayDialogsAndPopups()
{
	if (m_SaveDialogIfRequired)
		ShowSaveDialogIfRequired();
	
	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;

	if (m_ProjectFile.IsLoaded())
	{
		OptimizerPane::Instance()->DrawDialogAndPopups(&m_ProjectFile, min, max);
	}

	if (igfd::ImGuiFileDialog::Instance()->FileDialog("NewProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk)
		{
			NewProject(igfd::ImGuiFileDialog::Instance()->GetFilepathName());
		}

		igfd::ImGuiFileDialog::Instance()->CloseDialog("NewProjectDlg");
	}

	if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk)
		{
			LoadProject(igfd::ImGuiFileDialog::Instance()->GetFilepathName());
		}

		igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenProjectDlg");
	}

	if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk)
		{
			SaveAsProject(igfd::ImGuiFileDialog::Instance()->GetFilepathName());
		}

		igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveProjectDlg");
	}

	if (m_ShowAboutDialog)
		ShowAboutDialog(&m_ShowAboutDialog);
	if (m_ShowImGui)
		ImGui::ShowDemoWindow(&m_ShowImGui);
	if (m_ShowImGuiStyle)
		ImGui::ShowCustomStyleEditor(&m_ShowImGuiStyle);

	//SettingsDlg::Instance()->DrawDialog();
}

void MainFrame::ShowAboutDialog(bool *vOpen)
{
	ImGui::Begin("About GlslOptimizer", vOpen,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	// VERSION
	char buf[256];
	snprintf(buf, 255, "Version : GlslOptimizer %s", GLSLOPTIMIZER_VERSION);
	ImGui::ClickableTextUrl(buf, "https://github.com/aiekick/GlslOptimizer");

	ImGui::Separator();

	ImGui::Text("License :");
	ImGui::Indent();
	{
		ImGui::ClickableTextUrl(u8R"(Copyright 2020 Stephane Cuillerdier (aka Aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.)", "https://github.com/aiekick/GlslOptimizer/blob/master/LICENSE", false);
	}
	ImGui::Unindent();

	ImGui::Separator();

	ImGui::Text("My Accounts :");
	ImGui::Indent();
	{
		ImGui::ClickableTextUrl("Twitter", "https://twitter.com/aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Instagram", "https://www.instagram.com/aiekick/");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Sketchfab", "https://sketchfab.com/Aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Github", "https://github.com/aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("ArtStation", "https://www.artstation.com/aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Shadertoy", "https://www.shadertoy.com/user/aiekick");
	}
	ImGui::Unindent();

	ImGui::Separator();

	ImGui::Separator();

	ImGui::Text("Frameworks / Libraries / Api's used :");
	ImGui::Indent();
	{
		//glfw3
		ImGui::ClickableTextUrl("Glfw (ZLIB)", "http://www.glfw.org/");
		//ImGui
		ImGui::ClickableTextUrl("Dear ImGui (Docking branch) (MIT)", "https://github.com/ocornut/imgui");
		ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine();
		ImGui::ClickableTextUrl("Omar Cornut @Ocornut", "https://twitter.com/ocornut");
		//glad
		ImGui::ClickableTextUrl("Glad (MIT)", "https://github.com/Dav1dde/glad");
		//stb
		ImGui::ClickableTextUrl("Stb (MIT)", "https://github.com/nothings/stb");
		ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine();
		ImGui::ClickableTextUrl("Sean Barrett @Nothings", "https://twitter.com/nothings");
		//tinyxml2
		ImGui::ClickableTextUrl("tinyxml2 (ZLIB)", "https://github.com/leethomason/tinyxml2");
		//ImGuiColorTextEdit
		ImGui::ClickableTextUrl("ImGuiColorTextEdit (MIT)", "https://github.com/BalazsJako/ImGuiColorTextEdit");
		//dirent
		ImGui::ClickableTextUrl("dirent (MIT)", "https://github.com/tronkko/dirent/blob/master/include/dirent.h");
		//cTools
		ImGui::ClickableTextUrl("cTools (MIT)", "https://github.com/aiekick/cTools");
		//ImGuiFileDialog
		ImGui::ClickableTextUrl("ImGuiFileDialog (MIT)", "https://github.com/aiekick/ImGuiFileDialog");
	}
	ImGui::Unindent();

	ImGui::End();
}

void MainFrame::SetAppTitle(const std::string& vFilePathName)
{
	auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
	if (ps.isOk)
	{
		char bufTitle[1024];
		snprintf(bufTitle, 1023, "GlslOptimizer %s - Project : %s.glo", GLSLOPTIMIZER_VERSION, ps.name.c_str());
		glfwSetWindowTitle(m_Window, bufTitle);
	}
}

void MainFrame::ShowSaveDialogIfRequired()
{
	if (m_ProjectFile.IsLoaded())
	{
		if (m_ProjectFile.IsThereAnyNotSavedChanged())
		{
			bool choiceMade = false;

			ImGui::OpenPopup("Do you want to save before ?");
			if (ImGui::BeginPopupModal("Do you want to save before ?", (bool*)0,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
			{
				if (ImGui::Button("Save"))
				{
					choiceMade = true;
					m_SaveChangeDialogActions.push_front([this]()
					{
						SaveProject();
						m_SaveDialogIfRequired = false;
					});
					if (m_NeedToCloseApp)
					{
						m_SaveChangeDialogActions.push_front([this]()
						{
							glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
						});
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As"))
				{
					choiceMade = true;
					m_SaveChangeDialogActions.push_front([this]()
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", ".glo\0\0", ".");
						m_SaveDialogIfRequired = false;
					});
				}
				
				if (ImGui::Button("Continue without saving"))
				{
					choiceMade = true;
					if (m_NeedToCloseApp)
					{
						m_SaveChangeDialogActions.push_front([this]()
						{
							glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
						});
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					choiceMade = true;
					m_SaveChangeDialogActions.clear();
				}

				ImGui::EndPopup();

				if (choiceMade)
				{
					if (!m_SaveChangeDialogActions.empty())
					{
						for (auto &action : m_SaveChangeDialogActions)
						{
							action();
						}
					}
					else
					{
						m_SaveDialogIfRequired = false;
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::IWantToCloseTheApp()
{
	// some changes to save before closing
	if (m_ProjectFile.IsLoaded() && m_ProjectFile.IsThereAnyNotSavedChanged())
	{
		m_SaveDialogIfRequired = true;
		m_NeedToCloseApp = true;
	}
	else
	{
		glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrame::getXml(const std::string& vOffset)
{
	std::string str;

	str += ImGuiThemeHelper::Instance()->getXml(vOffset);
	str += GuiLayout::Instance()->getXml(vOffset);

	return str;
}

void MainFrame::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	ImGuiThemeHelper::Instance()->setFromXml(vElem, vParent);
	GuiLayout::Instance()->setFromXml(vElem, vParent);
}
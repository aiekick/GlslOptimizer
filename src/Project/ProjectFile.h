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
#include "tinyxml2/tinyxml2.h"
#include "ctools/GLVersionChecker.h"
#include "src/code/GlslConvert.h"
#include <imgui.h>

#include <string>

class ProjectFile : public conf::ConfigAbstract
{
public: // to save
	std::string m_ProjectFilePathName;
	std::string m_ProjectFilePath;
	GlslConvert::ShaderStage m_ShaderStage = GlslConvert::ShaderStage::MESA_SHADER_FRAGMENT;
	GlslConvert::ApiTarget m_ApiTarget = GlslConvert::ApiTarget::API_OPENGL_CORE;
	GlslConvert::LanguageTarget m_LanguageTarget = GlslConvert::LanguageTarget::LANGUAGE_TARGET_GLSL;
	GlslConvert::OptimizationStruct m_OptimizationStruct;
	
public: // dont save
	
private: // dont save
	bool m_IsLoaded = false;
	bool m_NeverSaved = false;
	bool m_IsThereAnyNotSavedChanged = false;

public:
	ProjectFile();
	explicit ProjectFile(const std::string& vFilePathName);
	~ProjectFile();

	void Clear();
	void New();
	void New(const std::string& vFilePathName);
	bool Load();
	bool LoadAs(const std::string& vFilePathName);
	bool Save();
	bool SaveAs(const std::string& vFilePathName);
	bool IsLoaded() const;

	bool IsThereAnyNotSavedChanged() const;
	void SetProjectChange(bool vChange = true);

	std::string GetAbsolutePath(const std::string& vFilePathName) const;
	std::string GetRelativePath(const std::string& vFilePathName) const;

public:
	std::string getXml(const std::string& vOffset) override;
	void setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent) override;

private:
	std::string getXml_From_OptimizationStruct(const std::string& vOffset);
	void setOptimizationStruct_From_Xml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent);

};


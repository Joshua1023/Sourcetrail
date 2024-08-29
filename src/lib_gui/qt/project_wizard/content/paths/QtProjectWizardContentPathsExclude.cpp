#include "QtProjectWizardContentPathsExclude.h"

#include "SourceGroupSettings.h"
#include "SourceGroupSettingsWithExcludeFilters.h"
#include "utility.h"
#include "utilityString.h"

QtProjectWizardContentPathsExclude::QtProjectWizardContentPathsExclude(
	std::shared_ptr<SourceGroupSettings> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPaths(
		  settings, window, QtPathListBox::SELECTION_POLICY_FILES_AND_DIRECTORIES, false)
{
	setTitleString(QStringLiteral("排除文件/文件夹"));
	setHelpString(
		"<p>这些路径定义了将从索引中排除的文件和文件夹。</p>"
		"<p>提示："
		"<ul>"
		"<li>您可以使用通配符 \"*\"，它代表除\"\\\" 或 \"/\" 以外的字符"
		"（例如 \"src/*/test.h\" 与 \"src/app/test.h\" 匹配，但不与"
		"\"src/app/widget/test.h\" 或 \"src/test.h\" 匹配）</li>"
		"<li>您可以使用通配符 \"**\" 来匹配任意字符 (例如 "
		"\"src**test.h\" 与 \"src/app/test.h\"、\"src/app/widget/test.h\" 或 "
		"\"src/test.h\" 都匹配)</li>"
		"<li>您可以通过 ${ENV_VAR} 的方式使用环境变量。</li>"
		"</ul></p>");
}

void QtProjectWizardContentPathsExclude::load()
{
	if (std::shared_ptr<SourceGroupSettingsWithExcludeFilters> settings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithExcludeFilters>(
				m_settings))	// FIXME: pass msettings as required type
	{
		m_list->setPaths(utility::convert<std::wstring, FilePath>(
			settings->getExcludeFilterStrings(), [](const std::wstring& s) { return FilePath(s); }));
	}
}

void QtProjectWizardContentPathsExclude::save()
{
	if (std::shared_ptr<SourceGroupSettingsWithExcludeFilters> settings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithExcludeFilters>(
				m_settings))	// FIXME: pass msettings as required type
	{
		settings->setExcludeFilterStrings(utility::toWStrings(m_list->getPathsAsDisplayed()));
	}
}

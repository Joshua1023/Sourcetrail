#include "QtProjectWizardContentPathsFrameworkSearch.h"

#include "SourceGroupSettings.h"
#include "SourceGroupSettingsWithCxxPathsAndFlags.h"

QtProjectWizardContentPathsFrameworkSearch::QtProjectWizardContentPathsFrameworkSearch(
	std::shared_ptr<SourceGroupSettings> settings,
	QtProjectWizardWindow* window,
	bool indicateAsAdditional)
	: QtProjectWizardContentPaths(
		  settings, window, QtPathListBox::SELECTION_POLICY_DIRECTORIES_ONLY, true)
{
	setTitleString(
		indicateAsAdditional ? QStringLiteral("附加框架搜索路径")
							 : QStringLiteral("框架搜索路径"));
	setHelpString(
		"框架搜索路径定义了您的项目所依赖的 MacOS 框架容器 (.framework) 的位置。这些路径通常使用“-iframework”标志传递给编译器。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。");
}

void QtProjectWizardContentPathsFrameworkSearch::load()
{
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> cxxSettings =
		std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(m_settings);
	if (cxxSettings)
	{
		m_list->setPaths(cxxSettings->getFrameworkSearchPaths());
	}
}

void QtProjectWizardContentPathsFrameworkSearch::save()
{
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> cxxSettings =
		std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(m_settings);
	if (cxxSettings)
	{
		cxxSettings->setFrameworkSearchPaths(m_list->getPathsAsDisplayed());
	}
}

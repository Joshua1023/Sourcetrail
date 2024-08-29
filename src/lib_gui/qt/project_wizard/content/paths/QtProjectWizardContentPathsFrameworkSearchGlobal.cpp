#include "QtProjectWizardContentPathsFrameworkSearchGlobal.h"

#include "ApplicationSettings.h"
#include "utilityApp.h"
#include "utilityPathDetection.h"

QtProjectWizardContentPathsFrameworkSearchGlobal::QtProjectWizardContentPathsFrameworkSearchGlobal(
	QtProjectWizardWindow* window)
	: QtProjectWizardContentPaths(
		  std::shared_ptr<SourceGroupSettings>(),
		  window,
		  QtPathListBox::SELECTION_POLICY_DIRECTORIES_ONLY,
		  true)
{
	setTitleString(QStringLiteral("全局框架搜索路径"));
	setHelpString(QString::fromStdString(
		"除了项目特定的框架搜索路径之外，全局框架搜索路径还将用于您的所有项目。<br />"
		"<br />"
		"它们定义了 MacOS 框架容器（.framework）的位置 "
		"（参考 <a href=\"" +
		utility::getDocumentationLink() +
		"#finding-system-header-locations\">"
		"查找系统头文件位置</a> 或使用下方的自动检测）。"));

	m_pathDetector = utility::getCxxFrameworkPathDetector();
	m_makePathsRelativeToProjectFileLocation = false;
}

void QtProjectWizardContentPathsFrameworkSearchGlobal::load()
{
	m_list->setPaths(ApplicationSettings::getInstance()->getFrameworkSearchPaths());
}

void QtProjectWizardContentPathsFrameworkSearchGlobal::save()
{
	ApplicationSettings::getInstance()->setFrameworkSearchPaths(m_list->getPathsAsDisplayed());
	ApplicationSettings::getInstance()->save();
}

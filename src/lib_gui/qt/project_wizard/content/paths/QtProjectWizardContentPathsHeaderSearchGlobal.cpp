#include "QtProjectWizardContentPathsHeaderSearchGlobal.h"
#include "QtMessageBox.h"

#include "ApplicationSettings.h"
#include "ResourcePaths.h"
#include "utilityApp.h"
#include "utilityPathDetection.h"

QtProjectWizardContentPathsHeaderSearchGlobal::QtProjectWizardContentPathsHeaderSearchGlobal(
	QtProjectWizardWindow* window)
	: QtProjectWizardContentPaths(
		  std::shared_ptr<SourceGroupSettings>(),
		  window,
		  QtPathListBox::SELECTION_POLICY_DIRECTORIES_ONLY,
		  true)
{
	setTitleString(QStringLiteral("全局 Include 路径"));
	setHelpString(QString::fromStdString(
		"除了项目特定的 include 路径外，全局 include 路径还将用于您的所有项目。这些路径通常使用“-isystem”标志传递给编译器。<br />"
		"<br />"
		"使用它们添加系统头文件路径（参考 <a "
		"href=\"" +
		utility::getDocumentationLink() +
		"#finding-system-header-locations\">"
		"查找系统头文件位置</a> 或者使用下方的自动检测）。"));

	m_pathDetector = utility::getCxxHeaderPathDetector();
	m_makePathsRelativeToProjectFileLocation = false;
}

void QtProjectWizardContentPathsHeaderSearchGlobal::load()
{
	setPaths(ApplicationSettings::getInstance()->getHeaderSearchPaths());
}

void QtProjectWizardContentPathsHeaderSearchGlobal::save()
{
	std::vector<FilePath> paths;
	for (const FilePath& headerPath: m_list->getPathsAsDisplayed())
	{
		if (headerPath != ResourcePaths::getCxxCompilerHeaderDirectoryPath())
		{
			paths.push_back(headerPath);
		}
	}

	ApplicationSettings::getInstance()->setHeaderSearchPaths(paths);
	ApplicationSettings::getInstance()->save();
}

bool QtProjectWizardContentPathsHeaderSearchGlobal::check()
{
	if constexpr (!utility::Platform::isWindows())
	{
		std::vector<FilePath> paths;
		QString compilerHeaderPaths;
		for (const FilePath& headerPath: m_list->getPathsAsDisplayed())
		{
			if (headerPath != ResourcePaths::getCxxCompilerHeaderDirectoryPath() &&
				headerPath.getCanonical().getConcatenated(L"/stdarg.h").exists())
			{
				compilerHeaderPaths += QString::fromStdWString(headerPath.wstr()) + "\n";
			}
			else
			{
				paths.push_back(headerPath);
			}
		}

		if (compilerHeaderPaths.size())
		{
			QtMessageBox msgBox(m_window);
			msgBox.setText(QStringLiteral("多编译器头文件"));
			msgBox.setInformativeText(
				"您的全局 include 路径包含其他 C/C++ 编译器头文件的路径，可能是您本地 C/C++ 编译器的路径。它们可能与 Sourcetrail 的 C/C++ 索引器的编译器头文件冲突，可能会导致索引期间出现兼容性错误。是否要移除这些路径？");
			msgBox.setDetailedText(compilerHeaderPaths);
			QPushButton *removeButton = msgBox.addButton(QStringLiteral("移除"), QtMessageBox::ButtonRole::YesRole);
			msgBox.addButton(QStringLiteral("保留"), QtMessageBox::ButtonRole::NoRole);
			msgBox.setIcon(QtMessageBox::Icon::Question);
			if (msgBox.execModal() == removeButton)	 // QtMessageBox::Yes
			{
				setPaths(paths);
				save();
			}
		}
	}
	return QtProjectWizardContentPaths::check();
}

void QtProjectWizardContentPathsHeaderSearchGlobal::detectedPaths(const std::vector<FilePath>& paths)
{
	std::vector<FilePath> headerPaths;
	for (const FilePath& headerPath: paths)
	{
		if (headerPath != ResourcePaths::getCxxCompilerHeaderDirectoryPath())
		{
			headerPaths.push_back(headerPath);
		}
	}
	setPaths(headerPaths);
}

void QtProjectWizardContentPathsHeaderSearchGlobal::setPaths(const std::vector<FilePath>& paths)
{
	// check data change to avoid UI update that messes with the scroll position
	{
		std::vector<FilePath> currentPaths = m_list->getPathsAsDisplayed();
		if (currentPaths.size())
		{
			currentPaths.erase(currentPaths.begin());
		}

		if (currentPaths.size() == paths.size())
		{
			bool same = true;
			for (size_t i = 0; i < paths.size(); ++i)
			{
				if (currentPaths[i] != paths[i])
				{
					same = false;
					break;
				}
			}

			if (same)
			{
				return;
			}
		}
	}

	m_list->setPaths({});
	m_list->addPaths({ResourcePaths::getCxxCompilerHeaderDirectoryPath()}, true);
	m_list->addPaths(paths);
}

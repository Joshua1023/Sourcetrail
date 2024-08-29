#include "QtProjectWizardContentPathsSource.h"
#include "QtMessageBox.h"

#include "language_packages.h"

#include "SourceGroupCustomCommand.h"
#include "SourceGroupSettingsCustomCommand.h"
#include "SourceGroupSettingsWithSourcePaths.h"
#include "utility.h"
#include "utilityFile.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
#	include "SourceGroupCxxEmpty.h"
#	include "SourceGroupSettingsWithCxxPathsAndFlags.h"
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE
#	include "SourceGroupJavaEmpty.h"
#	include "SourceGroupSettingsJavaEmpty.h"
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

#if BUILD_PYTHON_LANGUAGE_PACKAGE
#	include "SourceGroupPythonEmpty.h"
#	include "SourceGroupSettingsPythonEmpty.h"
#endif	  // BUILD_PYTHON_LANGUAGE_PACKAGE

QtProjectWizardContentPathsSource::QtProjectWizardContentPathsSource(
	std::shared_ptr<SourceGroupSettings> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPaths(
		  settings, window, QtPathListBox::SELECTION_POLICY_FILES_AND_DIRECTORIES, true)
{
	m_showFilesString = QStringLiteral("显示文件");

	setTitleString(QStringLiteral("要索引的文件/目录"));
	setHelpString(QStringLiteral(
		"这些路径定义了将被 Sourcetrail 索引的文件和目录。提供一个目录以递归添加所有包含的源文件和头文件。<br />"
		"<br />"
		"如果您的项目源代码位于一个位置，但生成的源文件保存在不同的位置，那么您也需要添加该目录。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。"));
	setIsRequired(true);
}

void QtProjectWizardContentPathsSource::load()
{
	if (std::shared_ptr<SourceGroupSettingsWithSourcePaths> pathSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourcePaths>(
				m_settings))	// FIXME: pass msettings as required type
	{
		m_list->setPaths(pathSettings->getSourcePaths());
	}
}

void QtProjectWizardContentPathsSource::save()
{
	if (std::shared_ptr<SourceGroupSettingsWithSourcePaths> pathSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourcePaths>(
				m_settings))	// FIXME: pass msettings as required type
	{
		pathSettings->setSourcePaths(m_list->getPathsAsDisplayed());
	}
}

bool QtProjectWizardContentPathsSource::check()
{
	if (m_list->getPathsAsDisplayed().empty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("您没有指定任何“要索引的文件/目录”。"));
		msgBox.setInformativeText(
			QStringLiteral("Sourcetrail 不会为该源文件组索引任何文件。请添加应索引的文件或目录的路径。"));
		QPushButton *continueButton = msgBox.addButton(QStringLiteral("继续"), QtMessageBox::ButtonRole::YesRole);
		msgBox.addButton(QStringLiteral("取消"), QtMessageBox::ButtonRole::NoRole);
		msgBox.setDefaultButton(continueButton);

		if (msgBox.execModal() != continueButton)
		{
			return false;
		}
	}

	return QtProjectWizardContentPaths::check();
}

std::vector<FilePath> QtProjectWizardContentPathsSource::getFilePaths() const
{
	std::set<FilePath> allSourceFilePaths;

#if BUILD_CXX_LANGUAGE_PACKAGE
	if (std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(m_settings))
	{
		allSourceFilePaths = SourceGroupCxxEmpty(m_settings).getAllSourceFilePaths();
	}
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE
	if (std::shared_ptr<SourceGroupSettingsJavaEmpty> settings =
			std::dynamic_pointer_cast<SourceGroupSettingsJavaEmpty>(m_settings))
	{
		allSourceFilePaths = SourceGroupJavaEmpty(settings).getAllSourceFilePaths();
	}
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

#if BUILD_PYTHON_LANGUAGE_PACKAGE
	if (std::shared_ptr<SourceGroupSettingsPythonEmpty> settings =
			std::dynamic_pointer_cast<SourceGroupSettingsPythonEmpty>(m_settings))
	{
		allSourceFilePaths = SourceGroupPythonEmpty(settings).getAllSourceFilePaths();
	}
#endif	  // BUILD_PYTHON_LANGUAGE_PACKAGE

	if (std::shared_ptr<SourceGroupSettingsCustomCommand> settings =
			std::dynamic_pointer_cast<SourceGroupSettingsCustomCommand>(m_settings))
	{
		allSourceFilePaths = SourceGroupCustomCommand(settings).getAllSourceFilePaths();
	}

	return utility::getAsRelativeIfShorter(
		utility::toVector(allSourceFilePaths), m_settings->getProjectDirectoryPath());
}

QString QtProjectWizardContentPathsSource::getFileNamesTitle() const
{
	return QStringLiteral("被索引文件");
}

QString QtProjectWizardContentPathsSource::getFileNamesDescription() const
{
	return QStringLiteral(" 个文件将被索引.");
}

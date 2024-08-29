#include "QtProjectWizardContentPathsClassJava.h"

#include <QCheckBox>

#include "SourceGroupSettings.h"
#include "SourceGroupSettingsWithClasspath.h"

QtProjectWizardContentPathsClassJava::QtProjectWizardContentPathsClassJava(
	std::shared_ptr<SourceGroupSettings> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPaths(
		  settings, window, QtPathListBox::SELECTION_POLICY_FILES_AND_DIRECTORIES, true)
{
	setTitleString("Class 路径");
	setHelpString(
		"输入项目所依赖的所有 .jar 文件。如果您的项目依赖不应编入索引的未编译 Java 代码，请在此处添加这些 .java 文件的根目录（所有包名都与之相关）。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。");
}

void QtProjectWizardContentPathsClassJava::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPaths::populate(layout, row);

	QLabel* label = createFormLabel("JRE 系统库");
	layout->addWidget(label, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignTop);

	m_useJreSystemLibraryCheckBox = new QCheckBox("使用 JRE 系统库", this);

	layout->addWidget(m_useJreSystemLibraryCheckBox, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentPathsClassJava::load()
{
	std::shared_ptr<SourceGroupSettingsWithClasspath> settings =
		std::dynamic_pointer_cast<SourceGroupSettingsWithClasspath>(m_settings);
	if (settings)
	{
		m_list->setPaths(settings->getClasspath());
		m_useJreSystemLibraryCheckBox->setChecked(settings->getUseJreSystemLibrary());
	}
}

void QtProjectWizardContentPathsClassJava::save()
{
	std::shared_ptr<SourceGroupSettingsWithClasspath> settings =
		std::dynamic_pointer_cast<SourceGroupSettingsWithClasspath>(m_settings);
	if (settings)
	{
		settings->setClasspath(m_list->getPathsAsDisplayed());
		settings->setUseJreSystemLibrary(m_useJreSystemLibraryCheckBox->isChecked());
	}
}

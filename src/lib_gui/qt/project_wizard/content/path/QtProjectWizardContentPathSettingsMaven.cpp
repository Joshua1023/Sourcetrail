#include "QtProjectWizardContentPathSettingsMaven.h"

#include <QCheckBox>

#include "SourceGroupSettingsJavaMaven.h"

QtProjectWizardContentPathSettingsMaven::QtProjectWizardContentPathSettingsMaven(
	std::shared_ptr<SourceGroupSettingsJavaMaven> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings)
{
	setTitleString("Maven 设置文件 (settings.xml)");
	setHelpString(
		"如果您的项目使用自定义 Maven 设置文件，请在此处指定。"
		"如果将此选项留空，则将使用默认的 Maven 设置。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。");
	setPlaceholderString("使用默认设置文件");
	setFileEndings({L".xml"});
}

void QtProjectWizardContentPathSettingsMaven::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);
	m_picker->setPickDirectory(false);
	m_picker->setFileFilter("Settings File (*.xml)");
}

void QtProjectWizardContentPathSettingsMaven::load()
{
	m_picker->setText(QString::fromStdWString(m_settings->getMavenSettingsFilePath().wstr()));
}

void QtProjectWizardContentPathSettingsMaven::save()
{
	m_settings->setMavenSettingsFilePath(FilePath(m_picker->getText().toStdWString()));
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathSettingsMaven::getSourceGroupSettings()
{
	return m_settings;
}

#include "QtProjectWizardContentCustomCommand.h"
#include "QtMessageBox.h"

#include <QCheckBox>
#include <QLineEdit>
#include <boost/filesystem/path.hpp>

#include "FileSystem.h"
#include "ProjectSettings.h"
#include "SourceGroupSettingsCustomCommand.h"
#include "SqliteIndexStorage.h"

QtProjectWizardContentCustomCommand::QtProjectWizardContentCustomCommand(
	std::shared_ptr<SourceGroupSettingsCustomCommand> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContent(window), m_settings(settings) 
{
}

void QtProjectWizardContentCustomCommand::populate(QGridLayout* layout, int& row)
{
	QLabel* nameLabel = createFormLabel(QStringLiteral("自定义命令"));
	addHelpButton(
		QStringLiteral("自定义命令"),
		"<p>指定为此源文件组中的每个源文件执行的命令行调用。"
		"你可以使用以下变量，其中必须使用 %{SOURCE_FILE_PATH}</p>"
		"<ul>"
		"<li><b>%{SOURCE_FILE_PATH}</b> - 每个源文件的路径（必须）</li>"
		"<li><b>%{DATABASE_FILE_PATH}</b> - 数据库文件路径: \"" +
			QString::fromStdWString(m_settings->getProjectSettings()->getTempDBFilePath().wstr()) +
			"\"</li>"
			"<li><b>%{DATABASE_VERSION}</b> - 数据库版本: "
			"\"" +
			QString::number(SqliteIndexStorage::getStorageVersion()) +
			"\"</li>"
			"<li><b>%{PROJECT_FILE_PATH}</b> - 项目文件路径: \"" +
			QString::fromStdWString(m_settings->getProjectSettings()->getProjectFilePath().wstr()) +
			"\"</li>"
			"</ul>",
		layout,
		row);

	m_customCommand = new QLineEdit();
	m_customCommand->setObjectName(QStringLiteral("name"));
	m_customCommand->setAttribute(Qt::WA_MacShowFocusRect, false);
	m_runInParallel = new QCheckBox(QStringLiteral("并行运行"));

	layout->setRowMinimumHeight(row, 30);

	layout->addWidget(nameLabel, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_customCommand, row, QtProjectWizardWindow::BACK_COL);
	row++;

	layout->addWidget(m_runInParallel, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentCustomCommand::load()
{
	m_customCommand->setText(QString::fromStdWString(m_settings->getCustomCommand()));
	m_runInParallel->setChecked(m_settings->getRunInParallel());
}

void QtProjectWizardContentCustomCommand::save()
{
	m_settings->setCustomCommand(m_customCommand->text().toStdWString());
	m_settings->setRunInParallel(m_runInParallel->isChecked());
}

bool QtProjectWizardContentCustomCommand::check()
{
	if (m_customCommand->text().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("请输入自定义命令"));
		msgBox.execModal();
		return false;
	}

	if (m_customCommand->text().toStdWString().find(L"%{SOURCE_FILE_PATH}") == std::wstring::npos)
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("自定义命令中缺少 %{SOURCE_FILE_PATH}。"));
		msgBox.execModal();
		return false;
	}

	return true;
}

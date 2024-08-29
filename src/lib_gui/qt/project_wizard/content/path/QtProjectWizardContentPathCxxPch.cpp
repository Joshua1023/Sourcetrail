#include "QtProjectWizardContentPathCxxPch.h"
#include "QtMessageBox.h"

#include "IndexerCommandCxx.h"
#include "SourceGroupSettingsCxxCdb.h"
#include "SourceGroupSettingsWithCxxPchOptions.h"
#include "utility.h"
#include "utilityFile.h"
#include "utilitySourceGroupCxx.h"

QtProjectWizardContentPathCxxPch::QtProjectWizardContentPathCxxPch(
	std::shared_ptr<SourceGroupSettings> settings,
	std::shared_ptr<SourceGroupSettingsWithCxxPchOptions> settingsCxxPch,
	QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings), m_settingsCxxPch(settingsCxxPch)
{
	setTitleString(QStringLiteral("预编译头文件"));
	setHelpString(
		"指定在索引之前用于生成预编译头的输入头文件的路径。<br />"
		"如果索引的源代码通常使用预编译头构建，则使用此选项将加快索引速度。<br />"
		"<br />"
		"如果您的源文件通过 \"#include &lt;pch.h&gt;\" 来使用预编译头，请指定 "
		"\"path/to/pch.h\".<br />"
		"<br />"
		"留空则禁用预编译头。您可以通过 ${ENV_VAR} 的方式使用环境变量。");
	setPlaceholderString(QStringLiteral("Not Using Precompiled Header"));
}

void QtProjectWizardContentPathCxxPch::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);

	m_picker->setPickDirectory(false);
}

void QtProjectWizardContentPathCxxPch::load()
{
	m_picker->setText(QString::fromStdWString(m_settingsCxxPch->getPchInputFilePath().wstr()));
}

void QtProjectWizardContentPathCxxPch::save()
{
	m_settingsCxxPch->setPchInputFilePathFilePath(FilePath(m_picker->getText().toStdWString()));
}

bool QtProjectWizardContentPathCxxPch::check()
{
	if (std::shared_ptr<SourceGroupSettingsCxxCdb> cdbSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsCxxCdb>(m_settings))
	{
		const FilePath cdbPath = cdbSettings->getCompilationDatabasePathExpandedAndAbsolute();
		std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb = utility::loadCDB(cdbPath);
		if (!cdb)
		{
			QtMessageBox msgBox(m_window);
			msgBox.setText(QStringLiteral("无法打开并读取提供的编译数据库文件。"));
			msgBox.execModal();
			return false;
		}

		if (utility::containsIncludePchFlags(cdb))
		{
			if (m_settingsCxxPch->getPchInputFilePath().empty())
			{
				QtMessageBox msgBox(m_window);
				msgBox.setText(
					"提供的编译数据库文件使用了预编译头。如果您想使用预编译头来加速索引器，请在预编译头文件中指定输入。");
				QPushButton* cancelButton = msgBox.addButton(QStringLiteral("取消"), QtMessageBox::ButtonRole::RejectRole);
				msgBox.addButton(QStringLiteral("继续"), QtMessageBox::ButtonRole::AcceptRole);
				return msgBox.execModal() != cancelButton;
			}
		}
		else
		{
			if (!m_settingsCxxPch->getPchInputFilePath().empty())
			{
				QtMessageBox msgBox(m_window);
				msgBox.setText(
					"提供的编译数据库文件未使用预编译头。不会使用预编译头文件中指定的输入文件。");
				QPushButton* cancelButton = msgBox.addButton(QStringLiteral("取消"), QtMessageBox::ButtonRole::RejectRole);
				msgBox.addButton(QStringLiteral("继续"), QtMessageBox::ButtonRole::AcceptRole);
				return msgBox.execModal() != cancelButton;
			}
		}
	}
	return true;
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathCxxPch::getSourceGroupSettings()
{
	return m_settings;
}

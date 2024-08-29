#include "QtProjectWizardContentPathPythonEnvironment.h"

#include "ResourcePaths.h"
#include "SourceGroupSettingsPythonEmpty.h"
#include "utilityApp.h"
#include "utilityFile.h"

QtProjectWizardContentPathPythonEnvironment::QtProjectWizardContentPathPythonEnvironment(
	std::shared_ptr<SourceGroupSettingsPythonEmpty> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings)
{
	setTitleString("Python 环境");
	setHelpString(
		"您可以在此处指定用于解析索引源代码中的依赖关系的目录路径或（虚拟）Python 环境的可执行文件路径。<br />"
		"<br />"
		"比如您运行过：<br />"
		"<br />"
		"$ cd C:\\dev\\python\\envs<br />"
		"$ virtualenv py37<br />"
		"<br />"
		"那么就可以将本项设置为 \"C:\\dev\\python\\envs\\py37\" 或 "
		"\"C:\\dev\\python\\envs\\py37\\Scripts\\python.exe\"。<br />"
		"留空则使用默认 Python 环境。您可以使用 ${ENV_VAR} 使用环境变量。");
	setPlaceholderString("使用默认环境");
}

void QtProjectWizardContentPathPythonEnvironment::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);
	connect(
		m_picker,
		&QtLocationPicker::textChanged,
		this,
		&QtProjectWizardContentPathPythonEnvironment::onTextChanged);

	m_resultLabel = new QLabel();
	m_resultLabel->setWordWrap(true);
	layout->addWidget(m_resultLabel, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentPathPythonEnvironment::load()
{
	m_picker->setText(QString::fromStdWString(m_settings->getEnvironmentPath().wstr()));
}

void QtProjectWizardContentPathPythonEnvironment::save()
{
	m_settings->setEnvironmentPath(FilePath(m_picker->getText().toStdWString()));
}

void QtProjectWizardContentPathPythonEnvironment::onTextChanged(const QString& text)
{
	if (text.isEmpty())
	{
		m_resultLabel->clear();
	}
	else
	{
		m_resultLabel->setText("正在检查 Python 环境的有效性...");
		std::thread([=, this]() {
			const utility::ProcessOutput out = utility::executeProcess(
				ResourcePaths::getPythonIndexerFilePath().wstr(),
				{L"check-environment",
				 L"--environment-path",
				 utility::getExpandedAndAbsolutePath(
					 FilePath(text.toStdWString()), m_settings->getProjectDirectoryPath())
					 .wstr()},
				FilePath(),
				false,
				boost::chrono::milliseconds(5000));
			m_onQtThread([=, this]() {
				if (out.exitCode == 0)
				{
					m_resultLabel->setText(QString::fromStdWString(out.output));
				}
				else
				{
					m_resultLabel->setText(
						"检查环境路径时出错。无法检查有效性。");
				}
			});
		}).detach();
	}
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathPythonEnvironment::getSourceGroupSettings()
{
	return m_settings;
}

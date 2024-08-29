#include "QtProjectWizardContentProjectData.h"
#include "QtMessageBox.h"

#include <QRegularExpression>
#include <boost/filesystem/path.hpp>

#include "FileSystem.h"
#include "ProjectSettings.h"

QtProjectWizardContentProjectData::QtProjectWizardContentProjectData(
	std::shared_ptr<ProjectSettings> projectSettings,
	QtProjectWizardWindow* window,
	bool disableNameEditing)
	: QtProjectWizardContent(window)
	, m_projectSettings(projectSettings)
	, m_disableNameEditing(disableNameEditing)
{
	setIsRequired(true);
}

void QtProjectWizardContentProjectData::populate(QGridLayout* layout, int& row)
{
	QLabel* nameLabel = createFormLabel(QStringLiteral("Sourcetrail 项目名"));
	m_projectName = new QLineEdit();
	m_projectName->setObjectName(QStringLiteral("name"));
	m_projectName->setAttribute(Qt::WA_MacShowFocusRect, false);
	m_projectName->setEnabled(!m_disableNameEditing);
	connect(
		m_projectName,
		&QLineEdit::textEdited,
		this,
		&QtProjectWizardContentProjectData::onProjectNameEdited);

	layout->addWidget(nameLabel, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_projectName, row, QtProjectWizardWindow::BACK_COL);
	layout->setRowMinimumHeight(row, 30);
	row++;

	QLabel* locationLabel = createFormLabel(QStringLiteral("Sourcetrail 项目位置"));
	m_projectFileLocation = new QtLocationPicker(this);
	m_projectFileLocation->setPickDirectory(true);
	m_projectFileLocation->setEnabled(!m_disableNameEditing);

	layout->addWidget(locationLabel, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_projectFileLocation, row, QtProjectWizardWindow::BACK_COL, Qt::AlignTop);
	addHelpButton(
		QStringLiteral("Sourcetrail 项目位置"),
		QStringLiteral("Sourcetrail 项目文件 (.srctrlprj) 将保存到该位置。"),
		layout,
		row);
	layout->setRowMinimumHeight(row, 30);
	row++;
}

void QtProjectWizardContentProjectData::load()
{
	m_projectName->setText(QString::fromStdWString(m_projectSettings->getProjectName()));
	m_projectFileLocation->setText(
		QString::fromStdWString(m_projectSettings->getProjectDirectoryPath().wstr()));
}

void QtProjectWizardContentProjectData::save()
{
	m_projectSettings->setProjectFilePath(
		m_projectName->text().toStdWString(),
		FilePath(m_projectFileLocation->getText().toStdWString()));
}

bool QtProjectWizardContentProjectData::check()
{
	if (m_projectName->text().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("请输入项目名"));
		msgBox.execModal();
		return false;
	}

	if (!boost::filesystem::portable_file_name(m_projectName->text().toStdString()))
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"提供的项目名不是有效的文件名。请相应调整名称。");
		msgBox.execModal();
		return false;
	}

	if (m_projectFileLocation->getText().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			QStringLiteral("请定义 Sourcetrail 项目文件的位置。"));
		msgBox.execModal();
		return false;
	}

	std::vector<FilePath> paths =
		FilePath(m_projectFileLocation->getText().toStdWString()).expandEnvironmentVariables();
	if (paths.size() != 1)
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"指定的位置似乎无效。请确保使用的环境变量没有歧义。");
		msgBox.execModal();
		return false;
	}
	else if (!paths.front().isAbsolute())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"指定的位置似乎无效。请指定绝对目录路径。");
		msgBox.execModal();
		return false;
	}
	else if (!paths.front().isValid())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"指定的位置似乎无效。请检查路径中使用的字符。");
		msgBox.execModal();
		return false;
	}
	else if (!paths[0].exists())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral(
			"指定的位置不存在。是否要创建目录？"));
		msgBox.addButton(QStringLiteral("否"), QtMessageBox::ButtonRole::NoRole);
		QPushButton* createButton = msgBox.addButton(
			QStringLiteral("是"), QtMessageBox::ButtonRole::YesRole);
		msgBox.setDefaultButton(createButton);
		msgBox.setIcon(QtMessageBox::Icon::Question);
		if (msgBox.execModal() == createButton)	 // QtMessageBox::Yes
		{
			FileSystem::createDirectories(paths[0]);
		}
		else
		{
			return false;
		}
	}

	return true;
}

void QtProjectWizardContentProjectData::onProjectNameEdited(QString text)
{
	const QRegularExpression regex(QStringLiteral("[^A-Za-z0-9_.-]"));

	const int cursorPosition = m_projectName->cursorPosition();

	text.replace(regex, QStringLiteral("_"));

	m_projectName->setText(text);
	m_projectName->setCursorPosition(cursorPosition);
}

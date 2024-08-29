#include "QtProjectWizardContentPathCodeblocksProject.h"

#include "QtProjectWizardContentPathsIndexedHeaders.h"
#include "SourceGroupCxxCodeblocks.h"
#include "SourceGroupSettingsCxxCodeblocks.h"
#include "utility.h"
#include "utilityFile.h"

QtProjectWizardContentPathCodeblocksProject::QtProjectWizardContentPathCodeblocksProject(
	std::shared_ptr<SourceGroupSettingsCxxCodeblocks> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings), m_filePaths([&]() {
		return utility::getAsRelativeIfShorter(
			utility::toVector(SourceGroupCxxCodeblocks(m_settings).getAllSourceFilePaths()),
			m_settings->getProjectDirectoryPath());
	})
{
	setTitleString(QStringLiteral("Code::Blocks 项目 (.cbp)"));
	setHelpString(
		"为项目选择 Code::Blocks 文件。Sourcetrail 将根据此文件的设置索引您的项目。它包含使用所需的所有 include 路径和编译器标志。Sourcetrail 项目将在每次更新索引时与 Code::Blocks 项目中的更改保持同步。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。");
	setFileEndings({L".cbp"});
	setIsRequired(true);
}

void QtProjectWizardContentPathCodeblocksProject::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);
	m_picker->setPickDirectory(false);
	m_picker->setFileFilter(QStringLiteral("Code::Blocks Project (*.cbp)"));
	connect(
		m_picker,
		&QtLocationPicker::locationPicked,
		this,
		&QtProjectWizardContentPathCodeblocksProject::pickedPath);

	QLabel* description = new QLabel(
		"Sourcetrail 将使用 Code::Blocks 项目中的所有设置，并在更新索引时保持最新的更改。",
		this);
	description->setObjectName(QStringLiteral("description"));
	description->setWordWrap(true);
	layout->addWidget(description, row, QtProjectWizardWindow::BACK_COL);
	row++;

	QLabel* title = createFormSubLabel(QStringLiteral("要索引的源文件"));
	layout->addWidget(title, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignTop);
	layout->setRowStretch(row, 0);

	m_fileCountLabel = new QLabel(QLatin1String(""));
	m_fileCountLabel->setWordWrap(true);
	layout->addWidget(m_fileCountLabel, row, QtProjectWizardWindow::BACK_COL, Qt::AlignTop);
	row++;

	addFilesButton(QStringLiteral("显示源文件"), layout, row);
	row++;
}

void QtProjectWizardContentPathCodeblocksProject::load()
{
	m_picker->setText(QString::fromStdWString(m_settings->getCodeblocksProjectPath().wstr()));

	m_filePaths.clear();

	if (m_fileCountLabel)
	{
		m_fileCountLabel->setText(
			"<b>" + QString::number(getFilePaths().size()) +
			"</b> 个源文件在 Code::Blocks 项目中被找到。");
	}
}

void QtProjectWizardContentPathCodeblocksProject::save()
{
	m_settings->setCodeblocksProjectPath(FilePath(m_picker->getText().toStdWString()));
}

std::vector<FilePath> QtProjectWizardContentPathCodeblocksProject::getFilePaths() const
{
	return m_filePaths.getValue();
}

QString QtProjectWizardContentPathCodeblocksProject::getFileNamesTitle() const
{
	return QStringLiteral("源文件");
}

QString QtProjectWizardContentPathCodeblocksProject::getFileNamesDescription() const
{
	return QStringLiteral(" 个源文件将被索引。");
}

void QtProjectWizardContentPathCodeblocksProject::pickedPath()
{
	m_window->saveContent();

	const FilePath projectPath = m_settings->getProjectDirectoryPath();

	std::set<FilePath> indexedHeaderPaths;
	for (const FilePath& path:
		 QtProjectWizardContentPathsIndexedHeaders::getIndexedPathsDerivedFromCodeblocksProject(
			 m_settings))
	{
		if (projectPath.contains(path))
		{
			indexedHeaderPaths.insert(path.getRelativeTo(
				projectPath));	  // the relative path is always shorter than the  absolute path
		}
	}
	m_settings->setIndexedHeaderPaths(utility::toVector(indexedHeaderPaths));

	m_window->loadContent();
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathCodeblocksProject::getSourceGroupSettings()
{
	return m_settings;
}

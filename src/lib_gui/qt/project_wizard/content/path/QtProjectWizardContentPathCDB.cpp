#include "QtProjectWizardContentPathCDB.h"

#include "QtProjectWizardContentPathsIndexedHeaders.h"
#include "SourceGroupCxxCdb.h"
#include "SourceGroupSettingsCxxCdb.h"
#include "utility.h"
#include "utilityFile.h"
#include "utilitySourceGroupCxx.h"

QtProjectWizardContentPathCDB::QtProjectWizardContentPathCDB(
	std::shared_ptr<SourceGroupSettingsCxxCdb> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings), m_filePaths([&]() {
		return utility::getAsRelativeIfShorter(
			utility::toVector(SourceGroupCxxCdb(m_settings).getAllSourceFilePaths()),
			m_settings->getProjectDirectoryPath());
	})
{
	setTitleString(QStringLiteral("编译数据库 (compile_commands.json)"));
	setHelpString(
		"选择项目的编译数据库文件。Sourcetrail 将根据编译命令为您的项目进行索引。此文件包含使用这些编译命令的所有 include 路径和编译器标志。每次更新索引时，项目都会与编译数据库中的更改保持同步。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。");
	setFileEndings({L".json"});
	setIsRequired(true);
}

void QtProjectWizardContentPathCDB::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);
	m_picker->setPickDirectory(false);
	m_picker->setFileFilter(QStringLiteral("JSON Compilation Database (*.json)"));
	connect(
		m_picker, &QtLocationPicker::locationPicked, this, &QtProjectWizardContentPathCDB::pickedPath);
	connect(
		m_picker,
		&QtLocationPicker::textChanged,
		this,
		&QtProjectWizardContentPathCDB::onPickerTextChanged);

	QLabel* description = new QLabel(
		"Sourcetrail 将使用编译数据库中的所有 include 路径和编译器标志，并在刷新时保持最新的更改。",
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

void QtProjectWizardContentPathCDB::load()
{
	m_picker->setText(QString::fromStdWString(m_settings->getCompilationDatabasePath().wstr()));

	refresh();
}

void QtProjectWizardContentPathCDB::save()
{
	m_settings->setCompilationDatabasePath(FilePath(m_picker->getText().toStdWString()));
}

void QtProjectWizardContentPathCDB::refresh()
{
	m_filePaths.clear();

	if (m_fileCountLabel)
	{
		m_fileCountLabel->setText(
			"<b>" + QString::number(getFilePaths().size()) +
			"</b> 个源文件在编译数据库被找到");
	}
}

std::vector<FilePath> QtProjectWizardContentPathCDB::getFilePaths() const
{
	return m_filePaths.getValue();
}

QString QtProjectWizardContentPathCDB::getFileNamesTitle() const
{
	return QStringLiteral("源文件");
}

QString QtProjectWizardContentPathCDB::getFileNamesDescription() const
{
	return QStringLiteral(" 个源文件将被索引。");
}

void QtProjectWizardContentPathCDB::pickedPath()
{
	m_window->saveContent();

	const FilePath projectPath = m_settings->getProjectDirectoryPath();

	std::set<FilePath> indexedHeaderPaths;
	for (const FilePath& path:
		 QtProjectWizardContentPathsIndexedHeaders::getIndexedPathsDerivedFromCDB(m_settings))
	{
		if (projectPath.contains(path))
		{
			// the relative path is always shorter than the absolute path
			indexedHeaderPaths.insert(path.getRelativeTo(projectPath));
		}
	}
	m_settings->setIndexedHeaderPaths(utility::toVector(indexedHeaderPaths));

	m_window->loadContent();
}

void QtProjectWizardContentPathCDB::onPickerTextChanged(const QString& text)
{
	const FilePath cdbPath = utility::getExpandedAndAbsolutePath(
		FilePath(text.toStdWString()), m_settings->getProjectDirectoryPath());
	if (!cdbPath.empty() && cdbPath.exists() &&
		cdbPath != m_settings->getCompilationDatabasePathExpandedAndAbsolute())
	{
		std::string error;
		std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb = utility::loadCDB(
			cdbPath, &error);
		if (cdb && error.empty())
		{
			pickedPath();
		}
	}
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathCDB::getSourceGroupSettings()
{
	return m_settings;
}

#include "QtProjectWizardContentPathsIndexedHeaders.h"
#include "QtMessageBox.h"

#include "CodeblocksProject.h"
#include "CompilationDatabase.h"
#include "IndexerCommandCxx.h"
#include "OrderedCache.h"
#include "QtPathListDialog.h"
#include "QtSelectPathsDialog.h"
#include "QtTextEditDialog.h"
#include "SourceGroupSettingsCxxCdb.h"
#include "SourceGroupSettingsCxxCodeblocks.h"
#include "logging.h"
#include "utility.h"
#include "utilityFile.h"

std::vector<FilePath> QtProjectWizardContentPathsIndexedHeaders::getIndexedPathsDerivedFromCodeblocksProject(
	std::shared_ptr<const SourceGroupSettingsCxxCodeblocks> settings)
{
	const FilePath projectPath = settings->getProjectDirectoryPath();
	std::set<FilePath> indexedHeaderPaths;
	{
		const FilePath codeblocksProjectPath = settings->getCodeblocksProjectPathExpandedAndAbsolute();
		if (!codeblocksProjectPath.empty() && codeblocksProjectPath.exists())
		{
			if (std::shared_ptr<Codeblocks::Project> codeblocksProject = Codeblocks::Project::load(
					codeblocksProjectPath))
			{
				OrderedCache<FilePath, FilePath> canonicalDirectoryPathCache(
					[](const FilePath& path) { return path.getCanonical(); });

				for (const FilePath& path: codeblocksProject->getAllSourceFilePathsCanonical(
						 settings->getSourceExtensions()))
				{
					indexedHeaderPaths.insert(
						canonicalDirectoryPathCache.getValue(path.getParentDirectory()));
				}
				utility::append(
					indexedHeaderPaths, codeblocksProject->getAllCxxHeaderSearchPathsCanonical());
			}
		}
		else
		{
			LOG_WARNING(
				"Unable to fetch indexed header paths. The provided Codeblocks project path does "
				"not exist.");
		}
	}

	std::vector<FilePath> topLevelPaths;
	for (const FilePath& path: utility::getTopLevelPaths(indexedHeaderPaths))
	{
		if (path.exists())
		{
			topLevelPaths.push_back(path);
		}
	}

	return topLevelPaths;
}

std::vector<FilePath> QtProjectWizardContentPathsIndexedHeaders::getIndexedPathsDerivedFromCDB(
	std::shared_ptr<const SourceGroupSettingsCxxCdb> settings)
{
	std::set<FilePath> indexedHeaderPaths;
	{
		const FilePath cdbPath = settings->getCompilationDatabasePathExpandedAndAbsolute();
		if (!cdbPath.empty() && cdbPath.exists())
		{
			for (const FilePath& path: IndexerCommandCxx::getSourceFilesFromCDB(cdbPath))
			{
				indexedHeaderPaths.insert(path.getCanonical().getParentDirectory());
			}
			for (const FilePath& path: utility::CompilationDatabase(cdbPath).getAllHeaderPaths())
			{
				if (path.exists())
				{
					indexedHeaderPaths.insert(path.getCanonical());
				}
			}
		}
		else
		{
			LOG_WARNING(
				"Unable to fetch indexed header paths. The provided Compilation Database path does "
				"not exist.");
		}
	}

	std::vector<FilePath> topLevelPaths;
	for (const FilePath& path: utility::getTopLevelPaths(indexedHeaderPaths))
	{
		if (path.exists())
		{
			topLevelPaths.push_back(path);
		}
	}

	return topLevelPaths;
}

QtProjectWizardContentPathsIndexedHeaders::QtProjectWizardContentPathsIndexedHeaders(
	std::shared_ptr<SourceGroupSettings> settings,
	QtProjectWizardWindow* window,
	const std::string& projectKindName)
	: QtProjectWizardContentPaths(
		  settings, window, QtPathListBox::SELECTION_POLICY_FILES_AND_DIRECTORIES, true)
	, m_projectKindName(projectKindName)
{
	m_showFilesString = QLatin1String("");

	setTitleString(QStringLiteral("要索引的头文件/目录"));
	setHelpString(QString::fromStdString(
		"当前项目已经指定了哪些源文件是项目的一部分。但 Sourcetrail 仍然需要知道哪些头文件作为项目的一部分进行索引以及哪些要跳过。选择跳过对系统头文件或外部框架进行索引将显著提高整体索引性能。<br />"
		"<br />"
		"使用此列表定义哪些头文件应由 Sourcetrail 索引。提供一个目录以递归添加所有包含的文件。<br />"
		"<br />"
		"您可以通过 ${ENV_VAR} 的方式使用环境变量。<br />"
		"<br />"
		"<b>提示</b>：如果您希望 Sourcetrail 索引其遇到的所有包含的头文件，只需输入项目的根目录。<br />"));
}

void QtProjectWizardContentPathsIndexedHeaders::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPaths::populate(layout, row);

	QPushButton* button = new QPushButton(QString::fromStdString("选择自 " + m_projectKindName));
	button->setObjectName(QStringLiteral("windowButton"));
	connect(
		button, &QPushButton::clicked, this, &QtProjectWizardContentPathsIndexedHeaders::buttonClicked);

	layout->addWidget(button, row, QtProjectWizardWindow::BACK_COL, Qt::AlignRight | Qt::AlignTop);
	row++;
}

void QtProjectWizardContentPathsIndexedHeaders::load()
{
	if (std::shared_ptr<SourceGroupSettingsWithIndexedHeaderPaths> cdbSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithIndexedHeaderPaths>(m_settings))
	{
		m_list->setPaths(cdbSettings->getIndexedHeaderPaths());
	}
}

void QtProjectWizardContentPathsIndexedHeaders::save()
{
	if (std::shared_ptr<SourceGroupSettingsWithIndexedHeaderPaths> cdbSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithIndexedHeaderPaths>(m_settings))
	{
		cdbSettings->setIndexedHeaderPaths(m_list->getPathsAsDisplayed());
	}
}

bool QtProjectWizardContentPathsIndexedHeaders::check()
{
	if (m_list->getPathsAsDisplayed().empty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			QStringLiteral("您没有指定任何要索引的头文件和目录。"));
		msgBox.setInformativeText(QString::fromStdString(
			"Sourcetrail 将仅索引项目文件中列出的源文件，而不会索引任何包含的头文件。"));
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

void QtProjectWizardContentPathsIndexedHeaders::buttonClicked()
{
	save();

	if (!m_filesDialog)
	{
		if (std::shared_ptr<SourceGroupSettingsCxxCodeblocks> codeblocksSettings =
				std::dynamic_pointer_cast<SourceGroupSettingsCxxCodeblocks>(m_settings))
		{
			const FilePath codeblocksProjectPath =
				codeblocksSettings->getCodeblocksProjectPathExpandedAndAbsolute();
			if (!codeblocksProjectPath.exists())
			{
				QtMessageBox msgBox(m_window);
				msgBox.setText(QStringLiteral("提供的 Code::Blocks 项目路径不存在。"));
				msgBox.setDetailedText(QString::fromStdWString(codeblocksProjectPath.wstr()));
				msgBox.execModal();
				return;
			}

			m_filesDialog = new QtSelectPathsDialog(
				"从 Include 路径中选择",
				"该列表包含在 Code::Blocks 项目中找到的所有 include 路径。红色路径不存在。选择包含要使用 Sourcetrail 索引的头文件的路径。",
				m_window);
			m_filesDialog->setup();

			connect(
				m_filesDialog,
				&QtSelectPathsDialog::finished,
				this,
				&QtProjectWizardContentPathsIndexedHeaders::savedFilesDialog);
			connect(
				m_filesDialog,
				&QtSelectPathsDialog::canceled,
				this,
				&QtProjectWizardContentPathsIndexedHeaders::closedFilesDialog);

			const FilePath projectPath = codeblocksSettings->getProjectDirectoryPath();

			dynamic_cast<QtSelectPathsDialog*>(m_filesDialog)
				->setPathsList(
					utility::convert<FilePath, FilePath>(
						getIndexedPathsDerivedFromCodeblocksProject(codeblocksSettings),
						[&](const FilePath& path) {
							return utility::getAsRelativeIfShorter(path, projectPath);
						}),
					codeblocksSettings->getIndexedHeaderPaths(),
					m_settings->getProjectDirectoryPath());
		}
		else if (
			std::shared_ptr<SourceGroupSettingsCxxCdb> cdbSettings =
				std::dynamic_pointer_cast<SourceGroupSettingsCxxCdb>(m_settings))
		{
			const FilePath cdbPath = cdbSettings->getCompilationDatabasePathExpandedAndAbsolute();
			if (!cdbPath.exists())
			{
				QtMessageBox msgBox(m_window);
				msgBox.setText(QStringLiteral("提供的编译数据库路径不存在。"));
				msgBox.setDetailedText(QString::fromStdWString(cdbPath.wstr()));
				msgBox.execModal();
				return;
			}

			m_filesDialog = new QtSelectPathsDialog(
				"从 Include 路径中选择",
				"该列表包含在编译数据库中找到的所有包含路径。红色路径不存在。选择包含要使用 Sourcetrail 索引的头文件的路径。",
				m_window);
			m_filesDialog->setup();

			connect(
				m_filesDialog,
				&QtSelectPathsDialog::finished,
				this,
				&QtProjectWizardContentPathsIndexedHeaders::savedFilesDialog);
			connect(
				m_filesDialog,
				&QtSelectPathsDialog::canceled,
				this,
				&QtProjectWizardContentPathsIndexedHeaders::closedFilesDialog);

			const FilePath projectPath = cdbSettings->getProjectDirectoryPath();

			dynamic_cast<QtSelectPathsDialog*>(m_filesDialog)
				->setPathsList(
					utility::convert<FilePath, FilePath>(
						getIndexedPathsDerivedFromCDB(cdbSettings),
						[&](const FilePath& path) {
							return utility::getAsRelativeIfShorter(path, projectPath);
						}),
					cdbSettings->getIndexedHeaderPaths(),
					m_settings->getProjectDirectoryPath());
		}
	}

	if (m_filesDialog)
	{
		m_filesDialog->showWindow();
		m_filesDialog->raise();
	}
}

void QtProjectWizardContentPathsIndexedHeaders::savedFilesDialog()
{
	m_list->setPaths(dynamic_cast<QtSelectPathsDialog*>(m_filesDialog)->getPathsList());
	closedFilesDialog();
}

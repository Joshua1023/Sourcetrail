#include "QtProjectWizardContentPathsHeaderSearch.h"
#include "QtMessageBox.h"

#include <cmath>

#include "Application.h"
#include "ApplicationSettings.h"
#include "FileManager.h"
#include "IncludeDirective.h"
#include "IncludeProcessing.h"
#include "QtDialogView.h"
#include "QtPathListDialog.h"
#include "QtTextEditDialog.h"
#include "ScopedFunctor.h"
#include "SourceGroupSettings.h"
#include "SourceGroupSettingsWithCxxPathsAndFlags.h"
#include "SourceGroupSettingsWithExcludeFilters.h"
#include "SourceGroupSettingsWithSourceExtensions.h"
#include "SourceGroupSettingsWithSourcePaths.h"
#include "utility.h"
#include "utilityFile.h"

QtProjectWizardContentPathsHeaderSearch::QtProjectWizardContentPathsHeaderSearch(
	std::shared_ptr<SourceGroupSettings> settings,
	QtProjectWizardWindow* window,
	bool indicateAsAdditional)
	: QtProjectWizardContentPaths(settings, window, QtPathListBox::SELECTION_POLICY_DIRECTORIES_ONLY, true)
	, m_showDetectedIncludesResultFunctor([this](const std::set<FilePath> &detectedHeaderSearchPaths)
		{ showDetectedIncludesResult(detectedHeaderSearchPaths); })
	, m_showValidationResultFunctor([this](const std::vector<IncludeDirective> &unresolvedIncludes)
		{ showValidationResult(unresolvedIncludes); })
	, m_indicateAsAdditional(indicateAsAdditional)
{
	setTitleString(m_indicateAsAdditional ? "附件 Include 路径" : "Include 路径");
	setHelpString(
		((m_indicateAsAdditional ? "<b>注意</b>：使用附加包含路径添加引用的项目文件中缺少的路径。<br /><br />"
								 : "") +
		 std::string("Include 路径用于解析索引源文件和头文件中的 #include 指令。这些路径通常使用“-I”或“-iquote”标志传递给编译器。<br />"
					 "<br />"
					 "添加整个项目中所有与 #include 指令相关的路径。如果所有 #include 指令都是相对于项目的根目录指定的，请在此处添加该根目录。<br />"
					 "<br />"
					 "如果您的项目还包含来自外部库的文件（例如 boost），请也添加这些目录（例如添加 'path/to/boost_home/include'）。<br />"
					 "<br />"
					 "您可以通过 ${ENV_VAR} 的方式使用环境变量。"))
			.c_str());
}

void QtProjectWizardContentPathsHeaderSearch::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPaths::populate(layout, row);

	if (!m_indicateAsAdditional)
	{
		{
			QPushButton* detectionButton = new QPushButton(QStringLiteral("自动检测"));
			detectionButton->setObjectName(QStringLiteral("windowButton"));
			connect(
				detectionButton,
				&QPushButton::clicked,
				this,
				&QtProjectWizardContentPathsHeaderSearch::detectIncludesButtonClicked);
			layout->addWidget(
				detectionButton, row, QtProjectWizardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
		}
		{
			QPushButton* validationButton = new QPushButton(
				QStringLiteral("验证 include 指令"));
			validationButton->setObjectName(QStringLiteral("windowButton"));
			connect(
				validationButton,
				&QPushButton::clicked,
				this,
				&QtProjectWizardContentPathsHeaderSearch::validateIncludesButtonClicked);
			layout->addWidget(
				validationButton, row, QtProjectWizardWindow::BACK_COL, Qt::AlignRight | Qt::AlignTop);
		}
		row++;
	}
}

void QtProjectWizardContentPathsHeaderSearch::load()
{
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> cxxSettings =
		std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(m_settings);
	if (cxxSettings)
	{
		m_list->setPaths(cxxSettings->getHeaderSearchPaths());
	}
}

void QtProjectWizardContentPathsHeaderSearch::save()
{
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> cxxSettings =
		std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(m_settings);
	if (cxxSettings)
	{
		cxxSettings->setHeaderSearchPaths(m_list->getPathsAsDisplayed());
	}
}

void QtProjectWizardContentPathsHeaderSearch::detectIncludesButtonClicked()
{
	m_window->saveContent();

	m_pathsDialog = std::make_shared<QtPathListDialog>(
		"检测 Include 路径",
		"<p>自动在下面提供的路径中搜索头文件，以查找源代码中未解析的 include 指令的缺失 include 路径。</p>"
		"<p>默认情况下会搜索“要索引的文件/目录”，但是您可以根据需要添加其他路径，例如第三方库的目录。</p>",
		QtPathListBox::SELECTION_POLICY_DIRECTORIES_ONLY);

	m_pathsDialog->setup();
	m_pathsDialog->updateNextButton(QStringLiteral("开始"));
	m_pathsDialog->setCloseVisible(true);

	m_pathsDialog->setRelativeRootDirectory(m_settings->getProjectDirectoryPath());
	if (std::shared_ptr<SourceGroupSettingsWithSourcePaths> pathSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourcePaths>(
				m_settings))	// FIXME: pass msettings as required type
	{
		m_pathsDialog->setPaths(pathSettings->getSourcePaths(), true);
	}
	m_pathsDialog->showWindow();

	connect(
		m_pathsDialog.get(),
		&QtPathListDialog::finished,
		this,
		&QtProjectWizardContentPathsHeaderSearch::finishedSelectDetectIncludesRootPathsDialog);
	connect(
		m_pathsDialog.get(),
		&QtPathListDialog::canceled,
		this,
		&QtProjectWizardContentPathsHeaderSearch::closedPathsDialog);
}

void QtProjectWizardContentPathsHeaderSearch::validateIncludesButtonClicked()
{
	// TODO: regard Force Includes here, too!
	m_window->saveContent();

	std::thread([&]() {
		std::shared_ptr<SourceGroupSettingsWithSourceExtensions> extensionSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourceExtensions>(m_settings);
		std::shared_ptr<SourceGroupSettingsWithSourcePaths> pathSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourcePaths>(m_settings);
		std::shared_ptr<SourceGroupSettingsWithExcludeFilters> excludeFilterSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithExcludeFilters>(m_settings);

		if (extensionSettings && pathSettings &&
			excludeFilterSettings)	  // FIXME: pass msettings as required type
		{
			std::vector<IncludeDirective> unresolvedIncludes;
			{
				QtDialogView* dialogView = dynamic_cast<QtDialogView*>(
					Application::getInstance()->getDialogView(DialogView::UseCase::PROJECT_SETUP).get());

				std::set<FilePath> sourceFilePaths;
				std::vector<FilePath> indexedFilePaths;
				std::vector<FilePath> headerSearchPaths;

				{
					dialogView->setParentWindow(m_window);
					dialogView->showUnknownProgressDialog(L"处理中", L"正在收集源文件");
					ScopedFunctor dialogHider(
						[&dialogView]() { dialogView->hideUnknownProgressDialog(); });

					FileManager fileManager;
					fileManager.update(
						pathSettings->getSourcePathsExpandedAndAbsolute(),
						excludeFilterSettings->getExcludeFiltersExpandedAndAbsolute(),
						extensionSettings->getSourceExtensions());
					sourceFilePaths = fileManager.getAllSourceFilePaths();

					indexedFilePaths = pathSettings->getSourcePathsExpandedAndAbsolute();

					headerSearchPaths =
						ApplicationSettings::getInstance()->getHeaderSearchPathsExpanded();
					if (std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> cxxSettings =
							std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(
								m_settings))
					{
						utility::append(
							headerSearchPaths,
							cxxSettings->getHeaderSearchPathsExpandedAndAbsolute());
					}
				}
				{
					dialogView->setParentWindow(m_window);
					ScopedFunctor dialogHider([&dialogView]() { dialogView->hideProgressDialog(); });

					unresolvedIncludes = IncludeProcessing::getUnresolvedIncludeDirectives(
						sourceFilePaths,
						utility::toSet(indexedFilePaths),
						utility::toSet(headerSearchPaths),
						static_cast<size_t>(log2(sourceFilePaths.size())),
						[&](const float progress) {
							dialogView->showProgressDialog(
								L"正在处理",
								std::to_wstring(int(progress * sourceFilePaths.size())) + L" 个文件",
								int(progress * 100.0f));
						});
				}
			}
			m_showValidationResultFunctor(unresolvedIncludes);
		}
	}).detach();
}


void QtProjectWizardContentPathsHeaderSearch::finishedSelectDetectIncludesRootPathsDialog()
{
	// TODO: regard Force Includes here, too!
	const std::vector<FilePath> searchedPaths = m_settings->makePathsExpandedAndAbsolute(
		m_pathsDialog->getPaths());
	closedPathsDialog();

	std::thread([=, this]() {
		std::shared_ptr<SourceGroupSettingsWithSourceExtensions> extensionSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourceExtensions>(m_settings);
		std::shared_ptr<SourceGroupSettingsWithSourcePaths> pathSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithSourcePaths>(m_settings);
		std::shared_ptr<SourceGroupSettingsWithExcludeFilters> excludeFilterSettings =
			std::dynamic_pointer_cast<SourceGroupSettingsWithExcludeFilters>(m_settings);

		if (extensionSettings && pathSettings &&
			excludeFilterSettings)	  // FIXME: pass msettings as required type
		{
			std::set<FilePath> detectedHeaderSearchPaths;
			{
				QtDialogView* dialogView = dynamic_cast<QtDialogView*>(
					Application::getInstance()->getDialogView(DialogView::UseCase::PROJECT_SETUP).get());

				std::set<FilePath> sourceFilePaths;
				std::vector<FilePath> headerSearchPaths;
				{
					dialogView->setParentWindow(m_window);
					dialogView->showUnknownProgressDialog(L"处理中", L"正在收集源文件");
					ScopedFunctor dialogHider(
						[&dialogView]() { dialogView->hideUnknownProgressDialog(); });

					FileManager fileManager;
					fileManager.update(
						pathSettings->getSourcePathsExpandedAndAbsolute(),
						excludeFilterSettings->getExcludeFiltersExpandedAndAbsolute(),
						extensionSettings->getSourceExtensions());
					sourceFilePaths = fileManager.getAllSourceFilePaths();

					headerSearchPaths =
						ApplicationSettings::getInstance()->getHeaderSearchPathsExpanded();
					if (std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> cxxSettings =
							std::dynamic_pointer_cast<SourceGroupSettingsWithCxxPathsAndFlags>(
								m_settings))
					{
						utility::append(
							headerSearchPaths,
							cxxSettings->getHeaderSearchPathsExpandedAndAbsolute());
					}
				}
				{
					dialogView->setParentWindow(m_window);
					ScopedFunctor dialogHider([&dialogView]() { dialogView->hideProgressDialog(); });

					detectedHeaderSearchPaths = IncludeProcessing::getHeaderSearchDirectories(
						sourceFilePaths,
						utility::toSet(searchedPaths),
						utility::toSet(headerSearchPaths),
						static_cast<size_t>(log2(sourceFilePaths.size())),
						[&](const float progress) {
							dialogView->showProgressDialog(
								L"正在处理",
								std::to_wstring(int(progress * sourceFilePaths.size())) + L" 个文件",
								int(progress * 100.0f));
						});
				}
			}

			m_showDetectedIncludesResultFunctor(detectedHeaderSearchPaths);
		}
	}).detach();
}

void QtProjectWizardContentPathsHeaderSearch::finishedAcceptDetectedIncludePathsDialog()
{
	const std::vector<std::wstring> detectedPaths = utility::split<std::vector<std::wstring>>(
		m_filesDialog->getText(), L"\n");
	closedFilesDialog();

	std::vector<FilePath> headerSearchPaths = m_list->getPathsAsDisplayed();

	headerSearchPaths.reserve(headerSearchPaths.size() + detectedPaths.size());
	for (const std::wstring& detectedPath: detectedPaths)
	{
		if (!detectedPath.empty())
		{
			headerSearchPaths.push_back(FilePath(detectedPath));
		}
	}

	m_list->setPaths(headerSearchPaths);
}

void QtProjectWizardContentPathsHeaderSearch::closedPathsDialog()
{
	m_pathsDialog->hide();
	m_pathsDialog.reset();

	window()->raise();
}

void QtProjectWizardContentPathsHeaderSearch::showDetectedIncludesResult(
	const std::set<FilePath>& detectedHeaderSearchPaths)
{
	const std::set<FilePath> headerSearchPaths = utility::toSet(
		m_settings->makePathsExpandedAndAbsolute(m_list->getPathsAsDisplayed()));

	std::vector<FilePath> additionalHeaderSearchPaths;
	{
		const FilePath relativeRoot = m_list->getRelativeRootDirectory();
		for (const FilePath& detectedHeaderSearchPath: detectedHeaderSearchPaths)
		{
			if (headerSearchPaths.find(detectedHeaderSearchPath) == headerSearchPaths.end())
			{
				additionalHeaderSearchPaths.push_back(
					utility::getAsRelativeIfShorter(detectedHeaderSearchPath, relativeRoot));
			}
		}
	}

	if (additionalHeaderSearchPaths.empty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"<p>搜索提供的路径时未检测到任何附加 include 路径。</p>");
		msgBox.execModal();
	}
	else
	{
		std::wstring detailedText;
		for (const FilePath& path: additionalHeaderSearchPaths)
		{
			detailedText += path.wstr() + L"\n";
		}

		m_filesDialog = new QtTextEditDialog(
			"检测到的 Include 路径",
			("<p>以下<b>" + std::to_string(additionalHeaderSearchPaths.size()) +
			 "</b> 条 include 路径已被检测到并将添加到该源文件组的 include 路径中。<b>")
				.c_str(),
			m_window);

		m_filesDialog->setup();
		m_filesDialog->setReadOnly(true);
		m_filesDialog->setCloseVisible(true);
		m_filesDialog->updateNextButton(QStringLiteral("添加"));

		m_filesDialog->setText(detailedText);
		m_filesDialog->showWindow();

		connect(
			m_filesDialog,
			&QtTextEditDialog::finished,
			this,
			&QtProjectWizardContentPathsHeaderSearch::finishedAcceptDetectedIncludePathsDialog);
		connect(
			m_filesDialog,
			&QtTextEditDialog::canceled,
			this,
			&QtProjectWizardContentPathsHeaderSearch::closedFilesDialog);
	}
}

void QtProjectWizardContentPathsHeaderSearch::showValidationResult(
	const std::vector<IncludeDirective>& unresolvedIncludes)
{
	if (unresolvedIncludes.empty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral(
			"<p>索引文件中的所有 include 指令均已解析。</p>"));
		msgBox.execModal();
	}
	else
	{
		std::map<std::wstring, std::map<size_t, std::wstring>> orderedIncludes;
		for (const IncludeDirective& unresolvedInclude: unresolvedIncludes)
		{
			orderedIncludes[unresolvedInclude.getIncludingFile().wstr()].emplace(
				unresolvedInclude.getLineNumber(), unresolvedInclude.getDirective());
		}

		std::wstring detailedText;
		for (const auto& p: orderedIncludes)
		{
			detailedText += p.first + L"\n";

			for (const auto& p2: p.second)
			{
				detailedText += std::to_wstring(p2.first) + L":\t" + p2.second + L"\n";
			}

			detailedText += L"\n";
		}

		m_filesDialog = new QtTextEditDialog(
			"未解析的 Include 指令",
			("<p>被索引文件中包含 <b>" + std::to_string(unresolvedIncludes.size()) +
			 "</b> 条未能正确解析的 include 指令。请检查详细信息并添加相应的头文件搜索路径。</p>"
			 "<p><b>注意</b>：这只是一次快速检查，不考虑块注释或条件预处理器指令。这意味着索引器实际上可能不需要某些未解析的 includes。</p>")
				.c_str(),
			m_window);

		m_filesDialog->setup();
		m_filesDialog->setCloseVisible(false);
		m_filesDialog->setReadOnly(true);

		m_filesDialog->setText(detailedText);
		m_filesDialog->showWindow();

		connect(
			m_filesDialog,
			&QtTextEditDialog::finished,
			this,
			&QtProjectWizardContentPathsHeaderSearch::closedFilesDialog);
		connect(
			m_filesDialog,
			&QtTextEditDialog::canceled,
			this,
			&QtProjectWizardContentPathsHeaderSearch::closedFilesDialog);
	}
}

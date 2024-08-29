#include "QtProjectWizardContentCxxPchFlags.h"
#include "QtMessageBox.h"

#include <QCheckBox>

#include "QtStringListBox.h"
#include "SourceGroupSettingsWithCxxPchOptions.h"

QtProjectWizardContentCxxPchFlags::QtProjectWizardContentCxxPchFlags(
	std::shared_ptr<SourceGroupSettingsWithCxxPchOptions> settings,
	QtProjectWizardWindow* window,
	bool isCDB)
	: QtProjectWizardContent(window), m_settings(settings), m_isCDB(isCDB)
{
}

void QtProjectWizardContentCxxPchFlags::populate(QGridLayout* layout, int& row)
{
	const QString labelText(QStringLiteral("预编译头标志"));
	layout->addWidget(
		createFormLabel(labelText), row, QtProjectWizardWindow::FRONT_COL, 2, 1, Qt::AlignTop);

	const QString optionText(
		m_isCDB ? QStringLiteral("使用第一个索引文件的标志以及'附加编译器标志'")
				: QStringLiteral("使用'编译器标志'"));

	const QString optionHelp(
		m_isCDB ? QStringLiteral("查看 <b>") + optionText +
				QStringLiteral("</b> 使用编译数据库的"
							   "第一个编译命令中指定的标志以及"
							   "'附加编译器标志'中指定的所有标志。")
				: QStringLiteral("查看 <b>") + optionText +
				QStringLiteral("</b> 以复用已指定的'编译器标志'。"));

	addHelpButton(
		QStringLiteral("预编译头标志"),
		QStringLiteral(
			"<p>定义预编译头文件生成期间使用的编译器标志。</p>"
			"<p>") +
			optionHelp +
			QStringLiteral(
				"</p>"
				"<p>另外，将编译器标志添加到仅用于预编译头生成的列表中，例如：</p>"
				"<p>* 使用 \"-DRELEASE\" 为 \"RELEASE\" 添加预处理器 #define</p>"
				"<p>* 使用 \"-U__clang__\" 为 \"__clang__\" 移除预处理器 #define</p>"),
		layout,
		row);

	m_useCompilerFlags = new QCheckBox(optionText);
	layout->addWidget(m_useCompilerFlags, row, QtProjectWizardWindow::BACK_COL);
	row++;

	m_list = new QtStringListBox(this, labelText);
	layout->addWidget(m_list, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentCxxPchFlags::load()
{
	m_useCompilerFlags->setChecked(m_settings->getUseCompilerFlags());
	m_list->setStrings(m_settings->getPchFlags());
}

void QtProjectWizardContentCxxPchFlags::save()
{
	m_settings->setUseCompilerFlags(m_useCompilerFlags->isChecked());
	m_settings->setPchFlags(m_list->getStrings());
}

bool QtProjectWizardContentCxxPchFlags::check()
{
	std::wstring error;

	for (const std::wstring& flag: m_list->getStrings())
	{
		if (utility::isPrefix<std::wstring>(L"-include ", flag) ||
			utility::isPrefix<std::wstring>(L"--include ", flag))
		{
			error = L"The entered flag \"" + flag +
				L"\" contains an error. Please remove the intermediate space character.\n";
		}
	}

	if (!error.empty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QString::fromStdWString(error));
		msgBox.execModal();
		return false;
	}

	return true;
}

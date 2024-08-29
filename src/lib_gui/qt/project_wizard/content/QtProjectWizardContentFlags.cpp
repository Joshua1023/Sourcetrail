#include "QtProjectWizardContentFlags.h"
#include "QtMessageBox.h"

#include <QFormLayout>

#include "QtStringListBox.h"
#include "SourceGroupSettingsWithCxxPathsAndFlags.h"

QtProjectWizardContentFlags::QtProjectWizardContentFlags(
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> settings,
	QtProjectWizardWindow* window,
	bool indicateAsAdditional)
	: QtProjectWizardContent(window)
	, m_settings(settings)
	, m_indicateAsAdditional(indicateAsAdditional)
{
}

void QtProjectWizardContentFlags::populate(QGridLayout* layout, int& row)
{
	const QString labelText(
		(std::string(m_indicateAsAdditional ? "附加" : "") + "编译标志").c_str());
	QLabel* label = createFormLabel(labelText);
	layout->addWidget(label, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignTop);

	addHelpButton(
		labelText,
		QStringLiteral(
			"<p>定义索引期间使用的其他 Clang 编译器标志。以下是一些示例：</p>"
			"<ul style=\"-qt-list-indent:0;\">"
			"<li style=\"margin-left:1em\">使用 \"-DRELEASE\" 为 \"RELEASE\" 添加预处理器 #define</li>"
			"<li style=\"margin-left:1em\">使用 \"-U__clang__\" 为 \"__clang__\" 移除预处理器 #define</li>"
			"<li style=\"margin-left:1em\">使用'-DFOO=900'添加整型预处理器定义</li>"
			"<li style=\"margin-left:1em\">使用'-DFOO=\"bar\"' 添加字符串预处理器定义</li>"
			"</ul>"),
		layout,
		row);

	m_list = new QtStringListBox(this, label->text());
	layout->addWidget(m_list, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentFlags::load()
{
	m_list->setStrings(m_settings->getCompilerFlags());
}

void QtProjectWizardContentFlags::save()
{
	m_settings->setCompilerFlags(m_list->getStrings());
}

bool QtProjectWizardContentFlags::check()
{
	std::wstring error;

	for (const std::wstring& flag: m_list->getStrings())
	{
		if (utility::isPrefix<std::wstring>(L"-include ", flag) ||
			utility::isPrefix<std::wstring>(L"--include ", flag))
		{
			error = L"输入的编译标志 \"" + flag +
				L"\" 含有错误。请删除中间的空格。\n";
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

#include "QtProjectWizardContentSourceGroupInfoText.h"

QtProjectWizardContentSourceGroupInfoText::QtProjectWizardContentSourceGroupInfoText(
	QtProjectWizardWindow* window)
	: QtProjectWizardContent(window)
{
}

void QtProjectWizardContentSourceGroupInfoText::populate(QGridLayout* layout, int& row)
{
	QHBoxLayout* layoutHorz = new QHBoxLayout();
	layout->addLayout(
		layoutHorz,
		row,
		QtProjectWizardWindow::FRONT_COL,
		1,
		1 + QtProjectWizardWindow::BACK_COL - QtProjectWizardWindow::FRONT_COL,
		Qt::AlignTop);

	layoutHorz->addSpacing(60);

	QLabel* infoLabel = new QLabel(
		"<p>请向您的项目添加至少一个源文件组。源文件组指定哪些源文件应由 Sourcetrail 分析，并包含分析这些源文件所需的所有参数。一个 Sourcetrail 项目可能包含多个源文件组，如果您想分析来自不同项目且不共享相同参数的源文件，这可能是必要的。</p>"
		"<p><b>提示</b>: 如果您的项目包含多个构建目标的源代码，您可以使用一个源文件组添加所有这些源文件，只要它们都共享相同的参数。</p>");
	infoLabel->setObjectName(QStringLiteral("info"));
	infoLabel->setWordWrap(true);
	layoutHorz->addWidget(infoLabel);

	layoutHorz->addSpacing(40);

	row++;
}
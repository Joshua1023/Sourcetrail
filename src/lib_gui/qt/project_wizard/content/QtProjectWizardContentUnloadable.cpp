#include "QtProjectWizardContentUnloadable.h"

#include "SourceGroupSettingsUnloadable.h"

QtProjectWizardContentUnloadable::QtProjectWizardContentUnloadable(
	std::shared_ptr<SourceGroupSettingsUnloadable> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContent(window), m_settings(settings)
{
}

void QtProjectWizardContentUnloadable::populate(QGridLayout* layout, int& row)
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

	QLabel* infoLabel = new QLabel(QString::fromStdString(
		"<p>此版本不支持所选的源文件组类型 \"" + m_settings->getTypeString() +
		"\"。</p>"));
	infoLabel->setObjectName(QStringLiteral("info"));
	infoLabel->setWordWrap(true);
	layoutHorz->addWidget(infoLabel);

	layoutHorz->addSpacing(40);

	row++;
}

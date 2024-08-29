#include "QtProjectWizardContentSourceGroupData.h"
#include "QtMessageBox.h"

#include <QCheckBox>
#include <QLineEdit>

#include "SourceGroupSettings.h"

QtProjectWizardContentSourceGroupData::QtProjectWizardContentSourceGroupData(
	std::shared_ptr<SourceGroupSettings> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContent(window), m_settings(settings) 
{
	setIsRequired(true);
}

void QtProjectWizardContentSourceGroupData::populate(QGridLayout* layout, int& row)
{
	m_name = new QLineEdit();
	m_name->setObjectName(QStringLiteral("name"));
	m_name->setAttribute(Qt::WA_MacShowFocusRect, false);
	connect(m_name, &QLineEdit::textEdited, this, &QtProjectWizardContentSourceGroupData::editedName);

	layout->addWidget(
		createFormLabel(QStringLiteral("源文件组名")),
		row,
		QtProjectWizardWindow::FRONT_COL,
		Qt::AlignRight);
	layout->addWidget(m_name, row, QtProjectWizardWindow::BACK_COL);
	row++;

	m_status = new QCheckBox(QStringLiteral("活动"));
	connect(
		m_status, &QCheckBox::toggled, this, &QtProjectWizardContentSourceGroupData::changedStatus);
	layout->addWidget(
		createFormSubLabel(QStringLiteral("状态")),
		row,
		QtProjectWizardWindow::FRONT_COL,
		Qt::AlignRight);
	layout->addWidget(m_status, row, QtProjectWizardWindow::BACK_COL);

	addHelpButton(
		QStringLiteral("状态"),
		"<p>索引期间将仅处理活动源文件组的源文件。不活动的源文件组将被忽略或从索引中清除。</p>"
		"<p>使用此设置可暂时从项目中删除文件（例如测试）。</p>",
		layout,
		row);

	row++;
}

void QtProjectWizardContentSourceGroupData::load()
{
	m_name->setText(QString::fromStdString(m_settings->getName()));

	m_status->setChecked(m_settings->getStatus() == SOURCE_GROUP_STATUS_ENABLED);
}

void QtProjectWizardContentSourceGroupData::save()
{
	m_settings->setName(m_name->text().toStdString());

	m_settings->setStatus(
		m_status->isChecked() ? SOURCE_GROUP_STATUS_ENABLED : SOURCE_GROUP_STATUS_DISABLED);
}

bool QtProjectWizardContentSourceGroupData::check()
{
	if (m_name->text().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("请输入源文件组名。"));
		msgBox.execModal();
		return false;
	}

	return true;
}

void QtProjectWizardContentSourceGroupData::editedName(QString name)
{
	if (!m_status->isChecked())
	{
		name = "(" + name + ")";
	}

	emit nameUpdated(name);
}

void QtProjectWizardContentSourceGroupData::changedStatus(bool  /*checked*/)
{
	emit statusUpdated(
		m_status->isChecked() ? SOURCE_GROUP_STATUS_ENABLED : SOURCE_GROUP_STATUS_DISABLED);

	editedName(m_name->text());
}

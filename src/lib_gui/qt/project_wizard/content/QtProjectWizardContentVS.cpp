#include "QtProjectWizardContentVS.h"

#include "MessageIDECreateCDB.h"
#include "utilityApp.h"

QtProjectWizardContentVS::QtProjectWizardContentVS(QtProjectWizardWindow* window)
	: QtProjectWizardContent(window)
{
}

void QtProjectWizardContentVS::populate(QGridLayout* layout, int& row)
{
	layout->setRowMinimumHeight(row++, 10);
	QLabel* nameLabel = createFormLabel(QStringLiteral("创建编译数据库"));
	layout->addWidget(nameLabel, row, QtProjectWizardWindow::FRONT_COL);

	addHelpButton(
		QStringLiteral("创建编译数据库"),
		QStringLiteral("要从 Visual Studio 解决方案创建新的编译数据库，必须在 Visual Studio 中打开一个解决方案。\n"
					   "Sourcetrail 将调用 Visual Studio 打开“创建编译数据库”对话框。请按照 Visual Studio 中的说明完成该过程。\n"
					   "注意: "
					   "必须安装 Sourcetrail 的 Visual Studio 插件。Visual Studio 必须与已加载的包含 C/C++ 项目的解决方案一起运行。"),
		layout,
		row);

	QLabel* descriptionLabel = createFormSubLabel(QString::fromStdString(
		"调用 Visual Studio 从已加载的解决方案中创建编译数据库（须安装相应插件）。"));
	descriptionLabel->setObjectName(QStringLiteral("description"));
	descriptionLabel->setOpenExternalLinks(true);
	descriptionLabel->setAlignment(Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(descriptionLabel, row, QtProjectWizardWindow::BACK_COL);
	row++;

	QPushButton* button = new QPushButton(QStringLiteral("创建编译数据库"));
	button->setObjectName(QStringLiteral("windowButton"));
	layout->addWidget(button, row, QtProjectWizardWindow::BACK_COL);
	row++;

	QLabel* skipLabel = createFormLabel(QStringLiteral(
		"*如果您的解决方案已有编译数据库，请跳过此步骤。"));
	skipLabel->setObjectName(QStringLiteral("description"));
	skipLabel->setAlignment(Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(skipLabel, row, QtProjectWizardWindow::BACK_COL);
	row++;

	layout->setRowMinimumHeight(row, 10);
	layout->setRowStretch(row, 1);

	connect(button, &QPushButton::clicked, this, &QtProjectWizardContentVS::handleVSCDBClicked);
}

void QtProjectWizardContentVS::handleVSCDBClicked()
{
	MessageIDECreateCDB().dispatch();
}

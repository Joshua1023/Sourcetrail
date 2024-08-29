#include "QtIndexingReportDialog.h"

#include <QLabel>
#include <QPushButton>

#include "MessageErrorsHelpMessage.h"
#include "MessageIndexingShowDialog.h"
#include "MessageRefresh.h"
#include "TimeStamp.h"

QtIndexingReportDialog::QtIndexingReportDialog(
	size_t indexedFileCount,
	size_t totalIndexedFileCount,
	size_t completedFileCount,
	size_t totalFileCount,
	float time,
	bool interrupted,
	bool shallow,
	QWidget* parent)
	: QtIndexingDialog(true, parent), m_interrupted(interrupted)
{
	setSizeGripStyle(false);

	if (interrupted)
	{
		QtIndexingDialog::createTitleLabel(QStringLiteral("中断索引"), m_layout);
	}
	else if (shallow)
	{
		QtIndexingDialog::createTitleLabel(QStringLiteral("完成浅索引"), m_layout);
	}
	else
	{
		QtIndexingDialog::createTitleLabel(QStringLiteral("完成索引"), m_layout);
	}

	m_layout->addSpacing(5);

	QtIndexingDialog::createMessageLabel(m_layout)->setText(
		QStringLiteral("被索引文件数：   ") + QString::number(indexedFileCount) + "/" +
		QString::number(totalIndexedFileCount));

	QtIndexingDialog::createMessageLabel(m_layout)->setText(
		QStringLiteral("处理完成文件总数：   ") + QString::number(completedFileCount) + "/" +
		QString::number(totalFileCount));

	m_layout->addSpacing(12);
	QtIndexingDialog::createMessageLabel(m_layout)->setText(
		QStringLiteral("花费时间：   ") + QString::fromStdString(TimeStamp::secondsToString(time)));

	m_layout->addSpacing(12);
	m_errorWidget = QtIndexingDialog::createErrorWidget(m_layout);

	m_layout->addStretch();

	if (shallow)
	{
		createMessageLabel(m_layout)->setText(QStringLiteral(
			"<i>您现在可以在再次运行深度索引的同时浏览您的项目！</i>"));
		m_layout->addSpacing(12);
	}

	{
		QHBoxLayout* buttons = new QHBoxLayout();
		if (interrupted)
		{
			QPushButton* discardButton = new QPushButton(QStringLiteral("废弃"));
			discardButton->setObjectName(QStringLiteral("windowButton"));
			connect(
				discardButton, &QPushButton::clicked, this, &QtIndexingReportDialog::onDiscardPressed);
			buttons->addWidget(discardButton);
		}
		else if (shallow)
		{
			QPushButton* startInDepthButton = new QPushButton(
				QStringLiteral("开始深度索引"));
			startInDepthButton->setObjectName(QStringLiteral("windowButton"));
			connect(
				startInDepthButton,
				&QPushButton::clicked,
				this,
				&QtIndexingReportDialog::onStartInDepthPressed);
			buttons->addWidget(startInDepthButton);
		}

		buttons->addStretch();

		QPushButton* confirmButton = new QPushButton(
			interrupted ? QStringLiteral("保留")
						: (shallow ? QStringLiteral("稍后处理") : QStringLiteral("好的")));
		confirmButton->setObjectName(QStringLiteral("windowButton"));
		confirmButton->setDefault(true);
		connect(
			confirmButton, &QPushButton::clicked, this, &QtIndexingReportDialog::onConfirmPressed);
		buttons->addWidget(confirmButton);

		m_layout->addLayout(buttons);
	}

	if (!interrupted)
	{
		QtIndexingDialog::createFlagLabel(this);
	}

	setupDone();
}

QSize QtIndexingReportDialog::sizeHint() const
{
	return QSize(m_interrupted ? 400 : 430, 280);
}

void QtIndexingReportDialog::updateErrorCount(size_t errorCount, size_t fatalCount)
{
	if (m_errorWidget && errorCount)
	{
		QString str = QString::number(errorCount) + QStringLiteral(" 个错误");

		if (fatalCount)
		{
			str += QStringLiteral("（") + QString::number(fatalCount) + QStringLiteral(" 致命错误）");
		}

		QPushButton* errorCount = m_errorWidget->findChild<QPushButton*>(
			QStringLiteral("errorCount"));
		errorCount->setText(str);

		m_errorWidget->show();
	}
}

void QtIndexingReportDialog::closeEvent(QCloseEvent*  /*event*/)
{
	emit QtIndexingDialog::canceled();
}

void QtIndexingReportDialog::keyPressEvent(QKeyEvent* event)
{
	if (!m_interrupted)	   // in this case we only show one button, so it is clear what to do
	{
		switch (event->key())
		{
		case Qt::Key_Escape:
		case Qt::Key_Return:
			onConfirmPressed();
			break;
		}
	}

	QWidget::keyPressEvent(event);
}

void QtIndexingReportDialog::onConfirmPressed()
{
	MessageErrorsHelpMessage().dispatch();

	emit QtIndexingDialog::finished();
}

void QtIndexingReportDialog::onDiscardPressed()
{
	emit QtIndexingDialog::canceled();
}

void QtIndexingReportDialog::onStartInDepthPressed()
{
	emit requestReindexing();
}

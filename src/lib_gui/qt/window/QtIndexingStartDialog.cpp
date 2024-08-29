#include "QtIndexingStartDialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>

#include "QtHelpButton.h"

QtIndexingStartDialog::QtIndexingStartDialog(
	const std::vector<RefreshMode>& enabledModes,
	const RefreshMode initialMode,
	bool enabledShallowOption,
	bool initialShallowState,
	QWidget* parent)
	: QtIndexingDialog(true, parent)
{
	setSizeGripStyle(false);

	QtIndexingDialog::createTitleLabel(QStringLiteral("开始索引"), m_layout);
	m_layout->addSpacing(5);

	m_clearLabel = QtIndexingDialog::createMessageLabel(m_layout);
	m_indexLabel = QtIndexingDialog::createMessageLabel(m_layout);

	m_clearLabel->setVisible(false);
	m_indexLabel->setVisible(false);

	m_layout->addStretch();

	QHBoxLayout* subLayout = new QHBoxLayout();
	subLayout->addStretch();

	QVBoxLayout* modeLayout = new QVBoxLayout();
	modeLayout->setSpacing(7);

	QHBoxLayout* modeTitleLayout = new QHBoxLayout();
	modeTitleLayout->setSpacing(7);

	QLabel* modeLabel = QtIndexingDialog::createMessageLabel(modeTitleLayout);
	modeLabel->setText(QStringLiteral("模式："));
	modeLabel->setAlignment(Qt::AlignLeft);

	QtHelpButton* helpButton = new QtHelpButton(QtHelpButtonInfo(
		QStringLiteral("索引模式"),
		QString("<b>增量更新：</b>重新索引自上次索引以来修改的所有文件、所有新文件以及所有依赖于这些文件的文件。<br /><br />"
				"<b>补充增量更新：</b>重新索引上次索引期间出现错误的所有文件、所有更新的文件以及所有依赖于这些文件的文件。<br /><br />"
				"<b>全量更新：</b>删除先前的索引并从头开始重新索引所有文件。<br /><br />") +
			(enabledShallowOption
				 ? "<br /><b>浅层 Python 索引：</b>通过名称解析代码库中的引用（调用、使用等），虽然不精确，但比深度索引快得多。<br />"
				   "<i>提示：使用此选项可以快速进行第一次索引，然后开始浏览代码库，同时运行第二次索引以进行深度索引。<br /><br />"
				 : "")));
	helpButton->setColor(Qt::white);
	modeTitleLayout->addWidget(helpButton);

	modeTitleLayout->addStretch();

	modeLayout->addLayout(modeTitleLayout);
	modeLayout->addSpacing(5);

	m_refreshModeButtons.emplace(
		REFRESH_UPDATED_FILES, new QRadioButton(QStringLiteral("增量更新")));
	m_refreshModeButtons.emplace(
		REFRESH_UPDATED_AND_INCOMPLETE_FILES,
		new QRadioButton(QStringLiteral("补充增量更新")));
	m_refreshModeButtons.emplace(REFRESH_ALL_FILES, new QRadioButton(QStringLiteral("全量更新")));

	std::function<void(bool)> func = [=, this](bool checked) {
		if (!checked)
		{
			return;
		}

		for (auto p: m_refreshModeButtons)
		{
			if (p.second->isChecked())
			{
				emit setMode(p.first);
				return;
			}
		}
	};

	for (auto p: m_refreshModeButtons)
	{
		QRadioButton* button = p.second;
		button->setObjectName(QStringLiteral("option"));
		button->setEnabled(false);
		if (p.first == initialMode)
		{
			button->setChecked(true);
		}
		modeLayout->addWidget(button);
		connect(button, &QRadioButton::toggled, func);
	}

	for (RefreshMode mode: enabledModes)
	{
		m_refreshModeButtons[mode]->setEnabled(true);
	}

	if (enabledShallowOption)
	{
		QCheckBox* shallowIndexingCheckBox = new QCheckBox(
			QStringLiteral("浅层 Python 索引"));
		connect(shallowIndexingCheckBox, &QCheckBox::toggled, [=, this]() {
			emit setShallowIndexing(shallowIndexingCheckBox->isChecked());
		});
		shallowIndexingCheckBox->setChecked(initialShallowState);
		modeLayout->addWidget(shallowIndexingCheckBox);
	}

	subLayout->addLayout(modeLayout);
	m_layout->addLayout(subLayout);

	m_layout->addSpacing(20);

	{
		QHBoxLayout* buttons = new QHBoxLayout();
		QPushButton* cancelButton = new QPushButton(QStringLiteral("取消"));
		cancelButton->setObjectName(QStringLiteral("windowButton"));
		connect(cancelButton, &QPushButton::clicked, this, &QtIndexingStartDialog::onCancelPressed);
		buttons->addWidget(cancelButton);

		buttons->addStretch();

		QPushButton* startButton = new QPushButton(QStringLiteral("开始"));
		startButton->setObjectName(QStringLiteral("windowButton"));
		startButton->setDefault(true);
		connect(startButton, &QPushButton::clicked, this, &QtIndexingStartDialog::onStartPressed);
		buttons->addWidget(startButton);
		m_layout->addLayout(buttons);
	}

	setupDone();
}

QSize QtIndexingStartDialog::sizeHint() const
{
	return QSize(350, 310);
}

void QtIndexingStartDialog::updateRefreshInfo(const RefreshInfo& info)
{
	QRadioButton* button = m_refreshModeButtons.find(info.mode)->second;
	if (!button->isChecked())
	{
		button->setChecked(true);
	}

	size_t clearCount = info.filesToClear.size();
	size_t indexCount = info.filesToIndex.size();

	m_clearLabel->setText("要清理的文件数：" + QString::number(clearCount));
	m_indexLabel->setText("要索引的源文件数：" + QString::number(indexCount));

	m_clearLabel->setVisible(clearCount && info.mode != REFRESH_ALL_FILES);
	m_indexLabel->setVisible(true);
}

void QtIndexingStartDialog::resizeEvent(QResizeEvent* event)
{
	QtIndexingDialog::resizeEvent(event);
}

void QtIndexingStartDialog::closeEvent(QCloseEvent*  /*event*/)
{
	emit QtIndexingDialog::canceled();
}

void QtIndexingStartDialog::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		onCancelPressed();
		break;
	case Qt::Key_Return:
		onStartPressed();
		break;
	}

	QWidget::keyPressEvent(event);
}

void QtIndexingStartDialog::onStartPressed()
{
	for (auto p: m_refreshModeButtons)
	{
		if (p.second->isChecked())
		{
			emit startIndexing(p.first);
			return;
		}
	}

	emit finished();
}

void QtIndexingStartDialog::onCancelPressed()
{
	emit canceled();
}

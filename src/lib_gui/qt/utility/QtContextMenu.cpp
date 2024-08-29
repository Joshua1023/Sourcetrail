#include "QtContextMenu.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

#include "MessageHistoryRedo.h"
#include "MessageHistoryUndo.h"
#include "logging.h"

QtContextMenu* QtContextMenu::s_instance;

QAction* QtContextMenu::s_undoAction;
QAction* QtContextMenu::s_redoAction;

QAction* QtContextMenu::s_copyFullPathAction;
QAction* QtContextMenu::s_openContainingFolderAction;

FilePath QtContextMenu::s_filePath;

QtContextMenu::QtContextMenu(QContextMenuEvent* event, QWidget* origin)
	: m_menu(origin), m_point(event->globalPos())
{
	getInstance();
}

void QtContextMenu::addAction(QAction* action)
{
	m_menu.addAction(action);
}

void QtContextMenu::enableUndo(bool enabled)
{
	s_undoAction->setEnabled(enabled);
}

void QtContextMenu::enableRedo(bool enabled)
{
	s_redoAction->setEnabled(enabled);
}

void QtContextMenu::addUndoActions()
{
	addAction(s_undoAction);
	addAction(s_redoAction);
}

void QtContextMenu::addFileActions(const FilePath& filePath)
{
	s_filePath = filePath;

	s_copyFullPathAction->setEnabled(!s_filePath.empty());
	s_openContainingFolderAction->setEnabled(!s_filePath.empty());

	addAction(s_copyFullPathAction);
	addAction(s_openContainingFolderAction);
}

QtContextMenu* QtContextMenu::getInstance()
{
	if (!s_instance)
	{
		s_instance = new QtContextMenu();

		s_undoAction = new QAction(tr("返回"), s_instance);
		s_undoAction->setStatusTip(tr("返回到上一活动符号"));
		s_undoAction->setToolTip(tr("返回到上一活动符号"));
		connect(s_undoAction, &QAction::triggered, s_instance, &QtContextMenu::undoActionTriggered);

		s_redoAction = new QAction(tr("前进"), s_instance);
		s_redoAction->setStatusTip(tr("前进到下一活动符号"));
		s_redoAction->setToolTip(tr("前进到下一活动符号"));
		connect(s_redoAction, &QAction::triggered, s_instance, &QtContextMenu::redoActionTriggered);

		s_copyFullPathAction = new QAction(tr("复制完整路径"), s_instance);
		s_copyFullPathAction->setStatusTip(tr("复制文件路径到剪贴板"));
		s_copyFullPathAction->setToolTip(tr("复制文件路径到剪贴板"));
		connect(
			s_copyFullPathAction,
			&QAction::triggered,
			s_instance,
			&QtContextMenu::copyFullPathActionTriggered);

		s_openContainingFolderAction = new QAction(tr("打开所在文件夹"), s_instance);
		s_openContainingFolderAction->setStatusTip(tr("打开文件所在文件夹"));
		s_openContainingFolderAction->setToolTip(tr("打开文件所在文件夹"));
		connect(
			s_openContainingFolderAction,
			&QAction::triggered,
			s_instance,
			&QtContextMenu::openContainingFolderActionTriggered);
	}

	return s_instance;
}

void QtContextMenu::addSeparator()
{
	m_menu.addSeparator();
}

void QtContextMenu::show()
{
	m_menu.exec(m_point);
}

void QtContextMenu::undoActionTriggered()
{
	MessageHistoryUndo().dispatch();
}

void QtContextMenu::redoActionTriggered()
{
	MessageHistoryRedo().dispatch();
}

void QtContextMenu::copyFullPathActionTriggered()
{
	QApplication::clipboard()->setText(
		QDir::toNativeSeparators(QString::fromStdWString(s_filePath.wstr())));
}

void QtContextMenu::openContainingFolderActionTriggered()
{
	FilePath dir = s_filePath.getParentDirectory();
	if (dir.exists())
	{
		QDesktopServices::openUrl(
			QUrl(QString::fromStdWString(L"file:///" + dir.wstr()), QUrl::TolerantMode));
	}
	else
	{
		LOG_ERROR(L"Unable to open directory: " + dir.wstr());
	}
}

QtContextMenu::QtContextMenu() = default;

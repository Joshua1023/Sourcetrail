#include "QtKeyboardShortcuts.h"

#include <QHeaderView>
#include <QLabel>

#include "ResourcePaths.h"
#include "utilityApp.h"
#include "utilityQt.h"

QtShortcutTable::QtShortcutTable(QWidget* parent): QTableWidget(parent) {}

void QtShortcutTable::updateSize()
{
	int height = rowCount() * rowHeight(0) + horizontalHeader()->height() + 2 * frameWidth() + 8;
	setMinimumHeight(height);
	setMaximumHeight(height);
}

void QtShortcutTable::wheelEvent(QWheelEvent* event)
{
	event->ignore();
}


QtKeyboardShortcuts::QtKeyboardShortcuts(QWidget* parent): QtWindow(false, parent)
{
	setScrollAble(true);
}

QtKeyboardShortcuts::~QtKeyboardShortcuts() = default;

QSize QtKeyboardShortcuts::sizeHint() const
{
	return QSize(666, 666);
}

void QtKeyboardShortcuts::populateWindow(QWidget* widget)
{
	QVBoxLayout* layout = new QVBoxLayout(widget);

	QLabel* generalLabel = new QLabel(this);
	generalLabel->setObjectName(QStringLiteral("general_label"));
	generalLabel->setText(QStringLiteral("常用快捷键"));
	layout->addWidget(generalLabel);

	layout->addWidget(createGeneralShortcutsTable());

	layout->addSpacing(20);

	QLabel* codeLabel = new QLabel(this);
	codeLabel->setObjectName(QStringLiteral("code_label"));
	codeLabel->setText(QStringLiteral("代码视图快捷键"));
	layout->addWidget(codeLabel);

	layout->addWidget(createCodeViewShortcutsTable());

	layout->addSpacing(20);

	QLabel* graphLabel = new QLabel(this);
	graphLabel->setObjectName(QStringLiteral("graph_label"));
	graphLabel->setText(QStringLiteral("图视图快捷键"));
	layout->addWidget(graphLabel);

	layout->addWidget(createGraphViewShortcutsTable());

	widget->setLayout(layout);

	widget->setStyleSheet(utility::getStyleSheet(ResourcePaths::getGuiDirectoryPath().concatenate(
													 L"keyboard_shortcuts/keyboard_shortcuts.css"))
							  .c_str());
}

void QtKeyboardShortcuts::windowReady()
{
	updateTitle(QStringLiteral("键盘快捷键"));
	updateCloseButton(QStringLiteral("关闭"));

	setNextVisible(false);
	setPreviousVisible(false);
}

QtKeyboardShortcuts::Shortcut::Shortcut(const QString& name, const QString& shortcut)
	: name(name), shortcut(shortcut)
{
}

QtKeyboardShortcuts::Shortcut QtKeyboardShortcuts::Shortcut::defaultOrMac(
	const QString& name, const QString& defaultShortcut, const QString& macShortcut)
{
	if constexpr (utility::Platform::isMac()) {
		return {name, macShortcut};
	} else {
		return {name, defaultShortcut};
	}
}

QtKeyboardShortcuts::Shortcut QtKeyboardShortcuts::Shortcut::winMacOrLinux(
	const QString& name,
	const QString& winShortcut,
	const QString& macShortcut,
	const QString& linuxShortcut)
{
	if constexpr (utility::Platform::isWindows()) {
		return {name, winShortcut};
	} else if constexpr (utility::Platform::isMac()) {
		return {name, macShortcut};
	} else {
		return {name, linuxShortcut};
	}
}

QtShortcutTable* QtKeyboardShortcuts::createTableWidget(const std::string& objectName)
{
	QtShortcutTable* table = new QtShortcutTable(this);
	table->setObjectName(objectName.c_str());

	table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

	table->setShowGrid(true);
	table->setAlternatingRowColors(true);
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	table->verticalHeader()->hide();

	table->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	table->setColumnCount(2);
	table->setHorizontalHeaderItem(0, new QTableWidgetItem(QStringLiteral("命令")));
	table->setHorizontalHeaderItem(1, new QTableWidgetItem(QStringLiteral("快捷键")));

	return table;
}

void QtKeyboardShortcuts::addShortcuts(QtShortcutTable* table, const std::vector<Shortcut>& shortcuts) 
{
	table->setRowCount(static_cast<int>(shortcuts.size()));

	for (size_t i = 0; i < shortcuts.size(); ++i)
	{
		table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(shortcuts[i].name));
		table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(shortcuts[i].shortcut));
	}

	table->updateSize();
}

QTableWidget* QtKeyboardShortcuts::createGeneralShortcutsTable()
{
	QtShortcutTable* table = createTableWidget("table_general");

	addShortcuts(
		table,
		{Shortcut(QStringLiteral("在图视图和代码视图间切换焦点"), QStringLiteral("Tab")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("增大字体"), QStringLiteral("Ctrl + +"), QStringLiteral("Cmd + +")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("减小字体"), QStringLiteral("Ctrl + -"), QStringLiteral("Cmd + -")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("重置字体大小"), QStringLiteral("Ctrl + 0"), QStringLiteral("Cmd + 0")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("返回"),
			 QStringLiteral("Alt + Left | Z | Y | Backspace"),
			 QStringLiteral("Cmd + [ | Z | Y | Backspace")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("前进"),
			 QStringLiteral("Alt + Right | Shift + Z | Shift + Y"),
			 QStringLiteral("Cmd + ] | Shift + Z | Shift + Y")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("更新索引"), QStringLiteral("F5"), QStringLiteral("Cmd + R")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("重新索引"),
			 QStringLiteral("Shift + F5"),
			 QStringLiteral("Cmd + Shift + R")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("符号查找"), QStringLiteral("Ctrl + F"), QStringLiteral("Cmd + F")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("文本查找"),
			 QStringLiteral("Ctrl + Shift + F"),
			 QStringLiteral("Cmd + Shift + F")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("当前页面查找"),
			 QStringLiteral("Ctrl + D | /"),
			 QStringLiteral("Cmd + D | /")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("新建项目"), QStringLiteral("Ctrl + N"), QStringLiteral("Cmd + N")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("打开项目"), QStringLiteral("Ctrl + O"), QStringLiteral("Cmd + O")),
		 Shortcut::winMacOrLinux(
			 QStringLiteral("退出"),
			 QStringLiteral("Alt + F4"),
			 QStringLiteral("Cmd + W"),
			 QStringLiteral("Ctrl + W")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("隐藏窗口"), QStringLiteral(""), QStringLiteral("Cmd + H")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("前往概览页"),
			 QStringLiteral("Ctrl + Home"),
			 QStringLiteral("Cmd + Home | Cmd + Up")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("设置"), QStringLiteral("Ctrl + ,"), QStringLiteral("Cmd + ,")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("为当前符号创建书签"),
			 QStringLiteral("Ctrl + S"),
			 QStringLiteral("Cmd + S")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("书签管理器"),
			 QStringLiteral("Ctrl + B"),
			 QStringLiteral("Cmd + B"))});

	return table;
}

QTableWidget* QtKeyboardShortcuts::createCodeViewShortcutsTable()
{
	QtShortcutTable* table = createTableWidget("table_code");

	addShortcuts(
		table,
		{Shortcut(QStringLiteral("在代码内移动焦点"), QStringLiteral("WASD | HJKL | Arrows")),
		 Shortcut(
			 QStringLiteral("将焦点移至最近的引用"),
			 QStringLiteral("Shift + WASD | Shift + HJKL | Shift + Arrows")),
		 Shortcut(QStringLiteral("激活位置"), QStringLiteral("Enter | E")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("在新标签页中激活位置"),
			 QStringLiteral("Ctrl + Shift + Enter | Ctrl + Shift + E"),
			 QStringLiteral("Cmd + Shift + Enter | Cmd + Shift + E")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("下一处引用"), QStringLiteral("Ctrl + G"), QStringLiteral("Cmd + G")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("上一处引用"),
			 QStringLiteral("Ctrl + Shift + G"),
			 QStringLiteral("Cmd + Shift + G")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("下一处本地引用"),
			 QStringLiteral("Ctrl + L"),
			 QStringLiteral("Cmd + L")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("上一处本地引用"),
			 QStringLiteral("Ctrl + Shift + L"),
			 QStringLiteral("Cmd + Shift + L")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("滚动代码区域"),
			 QStringLiteral("Ctrl + Arrows"),
			 QStringLiteral("Cmd + Arrows"))});

	return table;
}

QTableWidget* QtKeyboardShortcuts::createGraphViewShortcutsTable()
{
	QtShortcutTable* table = createTableWidget("table_graph");

	addShortcuts(
		table,
		{Shortcut(QStringLiteral("在节点内移动焦点"), QStringLiteral("WASD | HJKL | Arrows")),
		 Shortcut(
			 QStringLiteral("在边内移动焦点"),
			 QStringLiteral("Shift + WASD | Shift + HJKL | Shift + Arrows")),
		 Shortcut(QStringLiteral("激活节点/边"), QStringLiteral("Enter | E")),
		 Shortcut(QStringLiteral("展开/收起节点"), QStringLiteral("Shift + Enter | Shift + E")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("在新标签页激活节点"),
			 QStringLiteral("Ctrl + Shift + Enter | Ctrl + Shift + E"),
			 QStringLiteral("Cmd + Shift + Enter | Cmd + Shift + E")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("滚动图区域"),
			 QStringLiteral("Ctrl + Arrows"),
			 QStringLiteral("Cmd + Arrows")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("放大"),
			 QStringLiteral("Ctrl + Shift + Up | Ctrl + Mouse Wheel Up"),
			 QStringLiteral("Cmd + Shift + Up | Cmd + Mouse Wheel Up")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("缩小"),
			 QStringLiteral("Ctrl + Shift + Down | Ctrl + Mouse Wheel Down"),
			 QStringLiteral("Cmd + Shift + Down | Cmd + Mouse Wheel Down")),
		 Shortcut(QStringLiteral("重置"), QStringLiteral("0")),
		 Shortcut::defaultOrMac(
			 QStringLiteral("打开自定义踪迹对话框"),
			 QStringLiteral("Ctrl + U"),
			 QStringLiteral("Cmd + U"))});

	return table;
}

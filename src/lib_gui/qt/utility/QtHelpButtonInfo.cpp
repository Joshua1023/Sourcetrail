#include "QtHelpButtonInfo.h"
#include "QtMessageBox.h"

QtHelpButtonInfo::QtHelpButtonInfo(const QString& title, const QString& text)
	: m_title(title), m_text(text)
{
}

void QtHelpButtonInfo::displayMessage(QWidget* messageBoxParent)
{
	QtMessageBox msgBox(messageBoxParent);
	msgBox.setWindowTitle(QStringLiteral("Sourcetrail"));
	msgBox.setIcon(QtMessageBox::Information);
	msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
	msgBox.setText("<b>" + m_title + "</b>");
	msgBox.setInformativeText(m_text);
	msgBox.setStandardButtons(QtMessageBox::Ok);
	msgBox.setDefaultButton(QtMessageBox::Ok);
	msgBox.execModal();
}

QtHelpButtonInfo createErrorHelpButtonInfo()
{
	return QtHelpButtonInfo(
		QStringLiteral("修复错误"),
		QStringLiteral(
			"如果您的项目在索引后出现错误，请阅读此内容：<br />"
			"错误有多种类型："
			"<ul>"
			"<li><b>致命错误(Fatals)</b> 导致索引器停止。相关源文件的全部或大部分索引信息缺失。<b>请确保修复所有致命错误</b>。</li>"
			"<li><b>错误(Errors)</b> 是索引器认为是错误代码的问题。通常，索引器能够从这些错误中恢复。所涉及的源文件的索引信息可能不完整，但仍然有用。</li>"
			"</ul>"
			"您需要编辑 Sourcetrail 项目并重新索引以修复错误。显示的错误消息由 Sourcetrail 特定于语言的索引器生成："
			"<ul>"
			"<li><b>C/C++</b> 错误消息由<b>clang 编译器前端</b>生成。</li>"
			"<li><b>Java</b> 错误消息由 <b>Eclipse JDT 库</b> 生成。</li>"
			"<li><b>Python</b> 错误消息由<b>Jedi 静态分析库</b>生成。</li>"
			"</ul>"
			"您应该能够在网上找到有关您不熟悉的错误的信息。<b>双击</b>表中的一条错误消息以选择它进行<b>复制</b>。<br />"));
}

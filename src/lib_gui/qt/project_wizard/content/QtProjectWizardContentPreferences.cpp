#include "QtProjectWizardContentPreferences.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#include "ApplicationSettings.h"
#include "FileLogger.h"
#include "FileSystem.h"
#include "MessageSwitchColorScheme.h"
#include "TextCodec.h"
#include "ResourcePaths.h"
#include "logging.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityPathDetection.h"
#include "utilityQt.h"

using namespace utility;

QtProjectWizardContentPreferences::QtProjectWizardContentPreferences(QtProjectWizardWindow* window)
	: QtProjectWizardContent(window)
{
	m_colorSchemePaths = FileSystem::getFilePathsFromDirectory(
		ResourcePaths::getColorSchemesDirectoryPath(), {L".xml"});
}

QtProjectWizardContentPreferences::~QtProjectWizardContentPreferences()
{
	if (m_oldColorSchemeIndex != -1 && m_oldColorSchemeIndex != m_newColorSchemeIndex)
	{
		colorSchemeChanged(m_oldColorSchemeIndex);
	}
}

void QtProjectWizardContentPreferences::populate(QGridLayout* layout, int& row)
{
	ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();

	// ui
	addTitle(QStringLiteral("用户界面"), layout, row);

	// font face
	m_fontFacePlaceHolder = new QtComboBoxPlaceHolder();
	m_fontFace = new QFontComboBox();
	m_fontFace->setEditable(false);
	addLabelAndWidget(QStringLiteral("字体"), m_fontFacePlaceHolder, layout, row);

	int rowNum = row;
	connect(
		m_fontFacePlaceHolder,
		&QtComboBoxPlaceHolder::opened,
		[this, rowNum, layout]()
		{
			m_fontFacePlaceHolder->hide();

			QString name = m_fontFace->currentText();
			m_fontFace->setFontFilters(QFontComboBox::MonospacedFonts);
			m_fontFace->setWritingSystem(QFontDatabase::Latin);
			m_fontFace->setCurrentText(name);

			addWidget(m_fontFace, layout, rowNum);

			QTimer::singleShot(10, [this]() { m_fontFace->showPopup(); });
		});
	row++;

	// font size
	m_fontSize = addComboBox(
		QStringLiteral("字号"),
		appSettings->getFontSizeMin(),
		appSettings->getFontSizeMax(),
		QLatin1String(""),
		layout,
		row);

	// tab width
	m_tabWidth = addComboBox(QStringLiteral("标签宽度"), 1, 16, QLatin1String(""), layout, row);

	// text encoding
	m_textEncoding = addComboBox(QStringLiteral("文本编码"), QLatin1String(""), layout, row);
	m_textEncoding->addItems(TextCodec::availableCodecs());

	// color scheme
	m_colorSchemes = addComboBox(QStringLiteral("配色方案"), QLatin1String(""), layout, row);
	for (size_t i = 0; i < m_colorSchemePaths.size(); i++)
	{
		m_colorSchemes->insertItem(
			static_cast<int>(i),
			QString::fromStdWString(m_colorSchemePaths[i].withoutExtension().fileName()));
	}
	connect(
		m_colorSchemes,
		qOverload<int>(&QComboBox::activated),
		this,
		&QtProjectWizardContentPreferences::colorSchemeChanged);

	// animations
	m_useAnimations = addCheckBox(
		QStringLiteral("动画"),
		QStringLiteral("启动动画"),
		QStringLiteral("<p>在整个用户界面启用动画</p>"),
		layout,
		row);

	// built-in types
	m_showBuiltinTypes = addCheckBox(
		QStringLiteral("内置类型"),
		QStringLiteral("引用时在图(graph)中显示内置类型"),
		QStringLiteral("<p>引用时在图(graph)中显示内置类型</p>"),
		layout,
		row);

	// directory in code
	m_showDirectoryInCode = addCheckBox(
		QStringLiteral("显示文件相对路径"),
		QStringLiteral("在代码标题中显示文件相对路径"),
		QStringLiteral(
			"<p>在代码标题中显示文件相对路径</p>"),
		layout,
		row);
	layout->setRowMinimumHeight(row - 1, 30);

	addGap(layout, row);


	// Linux UI scale
	// TODO (PMost): Check https://doc.qt.io/qt-6/highdpi.html#environment-variable-reference
	if constexpr (utility::Platform::isLinux())
	{
		// screen
		addTitle(QStringLiteral("屏幕"), layout, row);

		QLabel* hint = new QLabel(QStringLiteral("<重启才能生效>"));
		hint->setStyleSheet(QStringLiteral("color: grey"));
		layout->addWidget(hint, row - 1, QtProjectWizardWindow::BACK_COL, Qt::AlignRight);

		// auto scaling
		m_screenAutoScalingInfoLabel = new QLabel(QLatin1String(""));
		m_screenAutoScaling = addComboBoxWithWidgets(
			QStringLiteral("自动缩放至 DPI"),
			QStringLiteral(
				"<p>定义自动缩放至屏幕 DPI 分辨率是否处于活动状态。"
				"此设置改变 Qt 框架的环境变量 QT_AUTO_SCREEN_SCALE_FACTOR"
				"(<a "
				"href=\"http://doc.qt.io/qt-5/highdpi.html\">http://doc.qt.io/qt-5/highdpi.html</"
				"a>)。"
				"选择'系统'和当前环境的设置保持一致。</p>"
				"<p>对此设置的更改需要重新启动程序才能生效。</p>"),
			{m_screenAutoScalingInfoLabel},
			layout,
			row);
		m_screenAutoScaling->addItem(QStringLiteral("系统"), -1);
		m_screenAutoScaling->addItem(QStringLiteral("关闭"), 0);
		m_screenAutoScaling->addItem(QStringLiteral("打开"), 1);
		connect(
			m_screenAutoScaling,
			qOverload<int>(&QComboBox::activated),
			this,
			&QtProjectWizardContentPreferences::uiAutoScalingChanges);

		// scale factor
		m_screenScaleFactorInfoLabel = new QLabel(QLatin1String(""));
		m_screenScaleFactor = addComboBoxWithWidgets(
			QStringLiteral("缩放系数"),
			QStringLiteral(
				"<p>为程序的用户界面定义屏幕缩放系数。"
				"此设置改变 Qt 框架的环境变量 QT_SCALE_FACTOR"
				"(<a "
				"href=\"http://doc.qt.io/qt-5/highdpi.html\">http://doc.qt.io/qt-5/highdpi.html</"
				"a>). "
				"选择'系统'和当前环境的设置保持一致。</p>"
				"<p>对此设置的更改需要重新启动程序才能生效。</p>"),
			{m_screenScaleFactorInfoLabel},
			layout,
			row);
		m_screenScaleFactor->addItem(QStringLiteral("系统"), -1.0);
		m_screenScaleFactor->addItem(QStringLiteral("25%"), 0.25);
		m_screenScaleFactor->addItem(QStringLiteral("50%"), 0.5);
		m_screenScaleFactor->addItem(QStringLiteral("75%"), 0.75);
		m_screenScaleFactor->addItem(QStringLiteral("100%"), 1.0);
		m_screenScaleFactor->addItem(QStringLiteral("125%"), 1.25);
		m_screenScaleFactor->addItem(QStringLiteral("150%"), 1.5);
		m_screenScaleFactor->addItem(QStringLiteral("175%"), 1.75);
		m_screenScaleFactor->addItem(QStringLiteral("200%"), 2.0);
		m_screenScaleFactor->addItem(QStringLiteral("250%"), 2.5);
		m_screenScaleFactor->addItem(QStringLiteral("300%"), 3.0);
		m_screenScaleFactor->addItem(QStringLiteral("400%"), 4.0);
		connect(
			m_screenScaleFactor,
			qOverload<int>(&QComboBox::activated),
			this,
			&QtProjectWizardContentPreferences::uiScaleFactorChanges);

		addGap(layout, row);
	}

	// Controls
	addTitle(QStringLiteral("控制"), layout, row);

	// scroll speed
	m_scrollSpeed = addLineEdit(
		QStringLiteral("滚动速度"),
		QStringLiteral(
			"<p>设置应用内滚动速度的倍数。</p>"
			"<p>0 到 1 之间的值会导致滚动速度变慢，而大于 1 的值则会加快滚动速度。</p>"),
		layout,
		row);

	// graph zooming
	QString modifierName = utility::Platform::isMac() ? QStringLiteral("Cmd") : QStringLiteral("Ctrl");
	m_graphZooming = addCheckBox(
		QStringLiteral("缩放"),
		QStringLiteral("使用鼠标滚轮缩放图(graph)"),
		QStringLiteral("<p>只需要使用鼠标滚轮即可对图(graph)进行缩放，无需 ") +
			modifierName + QStringLiteral(" + 鼠标滚轮。</p>"),
		layout,
		row);

	addGap(layout, row);

	// output
	addTitle(QStringLiteral("输出"), layout, row);

	// logging
	m_loggingEnabled = addCheckBox(
		QStringLiteral("日志"),
		QStringLiteral("启用控制台和文件日志记录"),
		QStringLiteral("<p>在控制台中显示日志并将此信息保存在文件中。</p>"),
		layout,
		row);
	connect(
		m_loggingEnabled,
		&QCheckBox::clicked,
		this,
		&QtProjectWizardContentPreferences::loggingEnabledChanged);

	m_verboseIndexerLoggingEnabled = addCheckBox(
		QStringLiteral("索引日志"),
		QStringLiteral("启用详细索引器日志记录"),
		QStringLiteral(
			"<p>在索引过程中启用抽象语法树遍历的附加日志。此信息可帮助追踪索引过程中发生的崩溃。</p>"
			"<p><b>警告</b>：这会大大降低索引性能。</p>"),
		layout,
		row);

	m_logPath = new QtLocationPicker(this);
	m_logPath->setPickDirectory(true);
	addLabelAndWidget(QStringLiteral("日志文件夹"), m_logPath, layout, row);
	addHelpButton(
		QStringLiteral("日志文件夹"),
		QStringLiteral("<p>日志文件会被保存在该路径下。</p>"),
		layout,
		row);
	row++;

	addGap(layout, row);

	// Plugins
	addTitle(QStringLiteral("插件"), layout, row);

	// Sourcetrail port
	m_sourcetrailPort = addLineEdit(
		QStringLiteral("Sourcetrail 端口"),
		QStringLiteral("<p>Sourcetrail 用于监听插件传入消息的端口号。</p>"),
		layout,
		row);

	// Sourcetrail port
	m_pluginPort = addLineEdit(
		QStringLiteral("插件端口"),
		QStringLiteral(
			"<p>Sourcetrail 用于向插件发送传出消息的端口号。</p>"),
		layout,
		row);

	addGap(layout, row);

	// indexing
	addTitle(QStringLiteral("索引"), layout, row);

	// indexer threads
	m_threadsInfoLabel = new QLabel(QLatin1String(""));
	utility::setWidgetRetainsSpaceWhenHidden(m_threadsInfoLabel);
	m_threads = addComboBoxWithWidgets(
		QStringLiteral("索引使用线程数"),
		0,
		24,
		QStringLiteral(
			"<p>设置用于并行索引项目的线程数。</p>"),
		{m_threadsInfoLabel},
		layout,
		row);
	m_threads->setItemText(0, QStringLiteral("默认"));
	connect(
		m_threads,
		qOverload<int>(&QComboBox::activated),
		this,
		&QtProjectWizardContentPreferences::indexerThreadsChanges);

	// multi process indexing
	m_multiProcessIndexing = addCheckBox(
		QStringLiteral("多进程<br />C/C++ 索引"),
		QStringLiteral("在不同进程中运行 C/C++ 索引线程"),
		QStringLiteral(
			"<p>使 C/C++ 索引器线程在不同的进程中运行。</p>"
			"<p>这可以防止程序因索引时出现不可预见的异常而崩溃。</p>"),
		layout,
		row);

	addGap(layout, row);


	// Java
	addTitle(QStringLiteral("JAVA"), layout, row);

	{
		// jvm library path
		m_javaPath = new QtLocationPicker(this);

		if constexpr (Platform::isWindows()) {
			m_javaPath->setFileFilter(QStringLiteral("JVM Library (jvm.dll)"));
			m_javaPath->setPlaceholderText(QStringLiteral("<jre_path>/bin/server/jvm.dll"));
		} else if constexpr (Platform::isMac()) {
			m_javaPath->setFileFilter(QStringLiteral("JVM Library (libjvm.dylib)"));
			m_javaPath->setPlaceholderText(QStringLiteral("<jre_path>/lib/server/libjvm.dylib"));
		} else if constexpr (Platform::isLinux()) {
			m_javaPath->setFileFilter(QStringLiteral("JVM Library (libjvm.so)"));
			m_javaPath->setPlaceholderText(QStringLiteral("<jre_path>/lib/server/libjvm.so"));
		} else
			LOG_WARNING("No placeholders and filters set for Java path selection");

		const std::string javaArchitectureString = utility::Platform::getArchitectureName();

		addLabelAndWidget(
			("Java 路径 (" + javaArchitectureString + ")").c_str(), m_javaPath, layout, row);

		const std::string javaVersionString = javaArchitectureString + " Java";

		addHelpButton(
			QStringLiteral("Java 路径"),
			("<p>只有索引 Java 项目才需要此选项。</p>"
			 "<p>提供 " +
			 javaVersionString +
			 " 运行时环境安装中 jvm 库的位置 (查看 "
			 "<a href=\"" +
			 utility::getDocumentationLink() +
			 "#finding-java-runtime-library-location\">"
			 "Finding Java Runtime Library Location</a> 获取更多信息或使用下方的自动检测)</p>")
				.c_str(),
			layout,
			row);
		row++;

		m_javaPathDetector = utility::getJavaRuntimePathDetector();
		addJavaPathDetection(layout, row);
	}

	{
		// JRE System Library
		const QString title = QStringLiteral("JRE 系统库");
		QLabel* label = createFormLabel(title);
		layout->addWidget(label, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignTop);

		addHelpButton(
			QStringLiteral("JRE 系统库"),
			QStringLiteral("<p>只有索引 Java 项目才需要此选项。</p>"
						   "<p>添加 JRE 系统库的 jar 文件。这些 jar 文件可以在 JRE 安装目录中找到。</p>"),
			layout,
			row);

		m_jreSystemLibraryPaths = new QtPathListBox(
			this, title, QtPathListBox::SELECTION_POLICY_FILES_ONLY);

		layout->addWidget(m_jreSystemLibraryPaths, row, QtProjectWizardWindow::BACK_COL);
		row++;

		m_jreSystemLibraryPathsDetector = utility::getJreSystemLibraryPathsDetector();
		addJreSystemLibraryPathsDetection(layout, row);
	}
	{
		// maven path
		m_mavenPath = new QtLocationPicker(this);

		if constexpr (Platform::isWindows()) {
			m_mavenPath->setFileFilter(QStringLiteral("Maven command (mvn.cmd)"));
			m_mavenPath->setPlaceholderText(QStringLiteral("<maven_path>/bin/mvn.cmd"));
		} else if constexpr (Platform::isLinux() || Platform::isMac()) {
			m_mavenPath->setFileFilter(QStringLiteral("Maven command (mvn)"));
			m_mavenPath->setPlaceholderText(QStringLiteral("<binarypath>/mvn"));
		} else
			LOG_WARNING("No placeholders and filters set for Maven path selection");

		addLabelAndWidget(QStringLiteral("Maven 路径"), m_mavenPath, layout, row);

		addHelpButton(
			QStringLiteral("Maven 路径"),
			QStringLiteral("<p>只有索引使用 Maven 的项目才需要此选项。</p>"
						   "<p>提供已安装 Maven 可执行文件的位置。也可以使用下方的自动检测。</p>"),
			layout,
			row);
		row++;

		m_mavenPathDetector = utility::getMavenExecutablePathDetector();
		addMavenPathDetection(layout, row);
	}

	addGap(layout, row);


	addTitle(QStringLiteral("Python"), layout, row);

	m_pythonPostProcessing = addCheckBox(
		QStringLiteral("后处理"),
		QStringLiteral("为未处理的引用添加歧义边（推荐）"),
		QStringLiteral("<p>索引完成后，启用后处理步骤来解决未处理的引用。"
					   "</p>"
					   "<p>这些引用将被标记为'歧义'，以表明其中一些边可能永远不会在索引代码的运行过程中遇到，因为后期处理仅依赖于符号名称和类型。</p>"),
		layout,
		row);

	addGap(layout, row);


	addTitle(QStringLiteral("C/C++"), layout, row);
}

void QtProjectWizardContentPreferences::load()
{
	ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();

	QString fontName = QString::fromStdString(appSettings->getFontName());
	m_fontFace->setCurrentText(fontName);
	m_fontFacePlaceHolder->addItem(fontName);
	m_fontFacePlaceHolder->setCurrentText(fontName);

	m_fontSize->setCurrentIndex(appSettings->getFontSize() - appSettings->getFontSizeMin());
	m_tabWidth->setCurrentIndex(appSettings->getCodeTabWidth() - 1);

	m_textEncoding->setCurrentText(QString::fromStdString(appSettings->getTextEncoding()));

	FilePath colorSchemePath = appSettings->getColorSchemePath();
	for (int i = 0; i < static_cast<int>(m_colorSchemePaths.size()); i++)
	{
		if (colorSchemePath == m_colorSchemePaths[i])
		{
			m_colorSchemes->setCurrentIndex(i);
			m_oldColorSchemeIndex = i;
			m_newColorSchemeIndex = i;
			break;
		}
	}

	m_useAnimations->setChecked(appSettings->getUseAnimations());
	m_showBuiltinTypes->setChecked(appSettings->getShowBuiltinTypesInGraph());
	m_showDirectoryInCode->setChecked(appSettings->getShowDirectoryInCodeFileTitle());

	if (m_screenAutoScaling)
	{
		m_screenAutoScaling->setCurrentIndex(
			m_screenAutoScaling->findData(appSettings->getScreenAutoScaling()));
		uiAutoScalingChanges(m_screenAutoScaling->currentIndex());
	}

	if (m_screenScaleFactor)
	{
		m_screenScaleFactor->setCurrentIndex(
			m_screenScaleFactor->findData(appSettings->getScreenScaleFactor()));
		uiScaleFactorChanges(m_screenScaleFactor->currentIndex());
	}

	m_scrollSpeed->setText(QString::number(appSettings->getScrollSpeed(), 'f', 1));
	m_graphZooming->setChecked(appSettings->getControlsGraphZoomOnMouseWheel());

	m_loggingEnabled->setChecked(appSettings->getLoggingEnabled());
	m_verboseIndexerLoggingEnabled->setChecked(appSettings->getVerboseIndexerLoggingEnabled());
	m_verboseIndexerLoggingEnabled->setEnabled(m_loggingEnabled->isChecked());
	if (m_logPath)
	{
		m_logPath->setText(QString::fromStdWString(appSettings->getLogDirectoryPath().wstr()));
	}

	m_sourcetrailPort->setText(QString::number(appSettings->getSourcetrailPort()));
	m_pluginPort->setText(QString::number(appSettings->getPluginPort()));

	m_threads->setCurrentIndex(
		appSettings->getIndexerThreadCount());	  // index and value are the same
	indexerThreadsChanges(m_threads->currentIndex());
	m_multiProcessIndexing->setChecked(appSettings->getMultiProcessIndexingEnabled());

	if (m_javaPath)
	{
		m_javaPath->setText(QString::fromStdWString(appSettings->getJavaPath().wstr()));
	}

	m_jreSystemLibraryPaths->setPaths(appSettings->getJreSystemLibraryPaths());

	if (m_mavenPath)
	{
		m_mavenPath->setText(QString::fromStdWString(appSettings->getMavenPath().wstr()));
	}

	m_pythonPostProcessing->setChecked(appSettings->getPythonPostProcessingEnabled());
}

void QtProjectWizardContentPreferences::save()
{
	std::shared_ptr<ApplicationSettings> appSettings = ApplicationSettings::getInstance();

	appSettings->setFontName(m_fontFace->currentText().toStdString());

	appSettings->setFontSize(m_fontSize->currentIndex() + appSettings->getFontSizeMin());
	appSettings->setCodeTabWidth(m_tabWidth->currentIndex() + 1);

	appSettings->setTextEncoding(m_textEncoding->currentText().toStdString());

	appSettings->setColorSchemeName(
		m_colorSchemePaths[m_colorSchemes->currentIndex()].withoutExtension().fileName());
	m_oldColorSchemeIndex = -1;

	appSettings->setUseAnimations(m_useAnimations->isChecked());
	appSettings->setShowBuiltinTypesInGraph(m_showBuiltinTypes->isChecked());
	appSettings->setShowDirectoryInCodeFileTitle(m_showDirectoryInCode->isChecked());

	if (m_screenAutoScaling)
	{
		appSettings->setScreenAutoScaling(m_screenAutoScaling->currentData().toInt());
	}

	if (m_screenScaleFactor)
	{
		appSettings->setScreenScaleFactor(m_screenScaleFactor->currentData().toFloat());
	}

	float scrollSpeed = m_scrollSpeed->text().toFloat();
	if (scrollSpeed)
		appSettings->setScrollSpeed(scrollSpeed);

	appSettings->setControlsGraphZoomOnMouseWheel(m_graphZooming->isChecked());

	appSettings->setLoggingEnabled(m_loggingEnabled->isChecked());
	appSettings->setVerboseIndexerLoggingEnabled(m_verboseIndexerLoggingEnabled->isChecked());
	if (m_logPath && m_logPath->getText().toStdWString() != appSettings->getLogDirectoryPath().wstr())
	{
		appSettings->setLogDirectoryPath(FilePath((m_logPath->getText() + '/').toStdWString()));
		Logger* logger = LogManager::getInstance()->getLoggerByType("FileLogger");
		if (logger)
		{
			auto *fileLogger = dynamic_cast<FileLogger*>(logger);
			fileLogger->setLogDirectory(appSettings->getLogDirectoryPath());
			fileLogger->setFileName(FileLogger::generateDatedFileName(L"log"));
		}
	}

	int sourcetrailPort = m_sourcetrailPort->text().toInt();
	if (sourcetrailPort)
		appSettings->setSourcetrailPort(sourcetrailPort);

	int pluginPort = m_pluginPort->text().toInt();
	if (pluginPort)
		appSettings->setPluginPort(pluginPort);

	appSettings->setIndexerThreadCount(m_threads->currentIndex());	  // index and value are the same
	appSettings->setMultiProcessIndexingEnabled(m_multiProcessIndexing->isChecked());

	if (m_javaPath)
	{
		appSettings->setJavaPath(FilePath(m_javaPath->getText().toStdWString()));
	}

	appSettings->setJreSystemLibraryPaths(m_jreSystemLibraryPaths->getPathsAsAbsolute());

	if (m_mavenPath)
	{
		appSettings->setMavenPath(FilePath(m_mavenPath->getText().toStdWString()));
	}

	appSettings->setPythonPostProcessingEnabled(m_pythonPostProcessing->isChecked());

	appSettings->save();
}

bool QtProjectWizardContentPreferences::check()
{
	return true;
}

void QtProjectWizardContentPreferences::colorSchemeChanged(int index)
{
	m_newColorSchemeIndex = index;
	MessageSwitchColorScheme(m_colorSchemePaths[index]).dispatch();
}

void QtProjectWizardContentPreferences::javaPathDetectionClicked()
{
	std::vector<FilePath> paths = m_javaPathDetector->getPathsForDetector(
		m_javaPathDetectorBox->currentText().toStdString());
	if (!paths.empty())
	{
		m_javaPath->setText(QString::fromStdWString(paths.front().wstr()));
	}
}

void QtProjectWizardContentPreferences::jreSystemLibraryPathsDetectionClicked()
{
	std::vector<FilePath> paths = m_jreSystemLibraryPathsDetector->getPathsForDetector(
		m_jreSystemLibraryPathsDetectorBox->currentText().toStdString());
	std::vector<FilePath> oldPaths = m_jreSystemLibraryPaths->getPathsAsAbsolute();
	m_jreSystemLibraryPaths->setPaths(utility::unique(utility::concat(oldPaths, paths)));
}

void QtProjectWizardContentPreferences::mavenPathDetectionClicked()
{
	std::vector<FilePath> paths = m_mavenPathDetector->getPathsForDetector(
		m_mavenPathDetectorBox->currentText().toStdString());
	if (!paths.empty())
	{
		m_mavenPath->setText(QString::fromStdWString(paths.front().wstr()));
	}
}

void QtProjectWizardContentPreferences::loggingEnabledChanged()
{
	m_verboseIndexerLoggingEnabled->setEnabled(m_loggingEnabled->isChecked());
}

void QtProjectWizardContentPreferences::indexerThreadsChanges(int index)
{
	if (index == 0)
	{
		m_threadsInfoLabel->setText(
			("合适的线程数为" + std::to_string(utility::getIdealThreadCount()))
				.c_str());
		m_threadsInfoLabel->show();
	}
	else
	{
		m_threadsInfoLabel->hide();
	}
}

void QtProjectWizardContentPreferences::uiAutoScalingChanges(int index)
{
	if (index == 0)
	{
		QString autoScale(qgetenv("QT_AUTO_SCREEN_SCALE_FACTOR_SOURCETRAIL"));
		if (autoScale == QLatin1String("1"))
		{
			autoScale = QStringLiteral("打开");
		}
		else
		{
			autoScale = QStringLiteral("关闭");
		}

		m_screenAutoScalingInfoLabel->setText(
			QStringLiteral("检测到: '") + autoScale + QStringLiteral("'"));
		m_screenAutoScalingInfoLabel->show();
	}
	else
	{
		m_screenAutoScalingInfoLabel->hide();
	}
}

void QtProjectWizardContentPreferences::uiScaleFactorChanges(int index)
{
	if (index == 0)
	{
		QString scale = QStringLiteral("100");
		bool ok;
		double scaleFactor = qgetenv("QT_SCALE_FACTOR_SOURCETRAIL").toDouble(&ok);
		if (ok)
		{
			scale = QString::number(int(scaleFactor * 100));
		}

		m_screenScaleFactorInfoLabel->setText(
			QStringLiteral("检测到: '") + scale + QStringLiteral("%'"));
		m_screenScaleFactorInfoLabel->show();
	}
	else
	{
		m_screenScaleFactorInfoLabel->hide();
	}
}

void QtProjectWizardContentPreferences::addJavaPathDetection(QGridLayout* layout, int& row)
{
	std::vector<std::string> detectorNames = m_javaPathDetector->getWorkingDetectorNames();
	if (detectorNames.empty())
	{
		return;
	}

	QLabel* label = new QLabel(QStringLiteral("自动检测自:"));

	m_javaPathDetectorBox = new QComboBox();

	for (const std::string& detectorName: detectorNames)
	{
		m_javaPathDetectorBox->addItem(detectorName.c_str());
	}

	QPushButton* button = new QPushButton(QStringLiteral("检测"));
	button->setObjectName(QStringLiteral("windowButton"));
	connect(
		button,
		&QPushButton::clicked,
		this,
		&QtProjectWizardContentPreferences::javaPathDetectionClicked);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(label);
	hlayout->addWidget(m_javaPathDetectorBox);
	hlayout->addWidget(button);

	QWidget* detectionWidget = new QWidget();
	detectionWidget->setLayout(hlayout);

	layout->addWidget(
		detectionWidget, row, QtProjectWizardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
	row++;
}

void QtProjectWizardContentPreferences::addJreSystemLibraryPathsDetection(QGridLayout* layout, int& row)
{
	const std::vector<std::string> detectorNames =
		m_jreSystemLibraryPathsDetector->getWorkingDetectorNames();
	if (detectorNames.empty())
	{
		return;
	}

	QLabel* label = new QLabel(QStringLiteral("自动检测自:"));

	m_jreSystemLibraryPathsDetectorBox = new QComboBox();

	for (const std::string& detectorName: detectorNames)
	{
		m_jreSystemLibraryPathsDetectorBox->addItem(detectorName.c_str());
	}

	QPushButton* button = new QPushButton(QStringLiteral("检测"));
	button->setObjectName(QStringLiteral("windowButton"));
	connect(
		button,
		&QPushButton::clicked,
		this,
		&QtProjectWizardContentPreferences::jreSystemLibraryPathsDetectionClicked);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(label);
	hlayout->addWidget(m_jreSystemLibraryPathsDetectorBox);
	hlayout->addWidget(button);

	QWidget* detectionWidget = new QWidget();
	detectionWidget->setLayout(hlayout);

	layout->addWidget(
		detectionWidget, row, QtProjectWizardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
	row++;
}

void QtProjectWizardContentPreferences::addMavenPathDetection(QGridLayout* layout, int& row)
{
	std::vector<std::string> detectorNames = m_mavenPathDetector->getWorkingDetectorNames();
	if (detectorNames.empty())
	{
		return;
	}

	QLabel* label = new QLabel(QStringLiteral("自动检测自:"));

	m_mavenPathDetectorBox = new QComboBox();

	for (const std::string& detectorName: detectorNames)
	{
		m_mavenPathDetectorBox->addItem(detectorName.c_str());
	}

	QPushButton* button = new QPushButton(QStringLiteral("检测"));
	button->setObjectName(QStringLiteral("windowButton"));
	connect(
		button,
		&QPushButton::clicked,
		this,
		&QtProjectWizardContentPreferences::mavenPathDetectionClicked);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(label);
	hlayout->addWidget(m_mavenPathDetectorBox);
	hlayout->addWidget(button);

	QWidget* detectionWidget = new QWidget();
	detectionWidget->setLayout(hlayout);

	layout->addWidget(
		detectionWidget, row, QtProjectWizardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
	row++;
}

void QtProjectWizardContentPreferences::addTitle(const QString& title, QGridLayout* layout, int& row)
{
	layout->addWidget(createFormTitle(title), row++, QtProjectWizardWindow::FRONT_COL, Qt::AlignLeft);
}

void QtProjectWizardContentPreferences::addLabel(const QString& label, QGridLayout* layout, int row)
{
	layout->addWidget(createFormLabel(label), row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
}

void QtProjectWizardContentPreferences::addWidget(
	QWidget* widget, QGridLayout* layout, int row, Qt::Alignment widgetAlignment)
{
	layout->addWidget(widget, row, QtProjectWizardWindow::BACK_COL, widgetAlignment);
}

void QtProjectWizardContentPreferences::addLabelAndWidget(
	const QString& label, QWidget* widget, QGridLayout* layout, int row, Qt::Alignment widgetAlignment)
{
	addLabel(label, layout, row);
	addWidget(widget, layout, row, widgetAlignment);
}

void QtProjectWizardContentPreferences::addGap(QGridLayout* layout, int& row)
{
	layout->setRowMinimumHeight(row++, 20);
}

QCheckBox* QtProjectWizardContentPreferences::addCheckBox(
	const QString& label, const QString& text, const QString& helpText, QGridLayout* layout, int& row)
{
	QCheckBox* checkBox = new QCheckBox(text, this);
	addLabelAndWidget(label, checkBox, layout, row, Qt::AlignLeft);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return checkBox;
}

QComboBox* QtProjectWizardContentPreferences::addComboBox(
	const QString& label, const QString& helpText, QGridLayout* layout, int& row)
{
	QComboBox* comboBox = new QComboBox(this);
	addLabelAndWidget(label, comboBox, layout, row, Qt::AlignLeft);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return comboBox;
}

QComboBox* QtProjectWizardContentPreferences::addComboBoxWithWidgets(
	const QString& label,
	const QString& helpText,
	std::vector<QWidget*> widgets,
	QGridLayout* layout,
	int& row)
{
	QComboBox* comboBox = new QComboBox(this);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(comboBox);

	for (QWidget* widget: widgets)
	{
		hlayout->addWidget(widget);
	}

	QWidget* container = new QWidget();
	container->setLayout(hlayout);

	addLabelAndWidget(label, container, layout, row, Qt::AlignLeft);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return comboBox;
}

QComboBox* QtProjectWizardContentPreferences::addComboBox(
	const QString& label, int min, int max, const QString& helpText, QGridLayout* layout, int& row)
{
	QComboBox* comboBox = addComboBox(label, helpText, layout, row);

	if (min != max)
	{
		for (int i = min; i <= max; i++)
		{
			comboBox->insertItem(i, QString::number(i));
		}
	}

	return comboBox;
}

QComboBox* QtProjectWizardContentPreferences::addComboBoxWithWidgets(
	const QString& label,
	int min,
	int max,
	const QString& helpText,
	std::vector<QWidget*> widgets,
	QGridLayout* layout,
	int& row)
{
	QComboBox* comboBox = addComboBoxWithWidgets(label, helpText, widgets, layout, row);

	if (min != max)
	{
		for (int i = min; i <= max; i++)
		{
			comboBox->insertItem(i, QString::number(i));
		}
	}

	return comboBox;
}

QLineEdit* QtProjectWizardContentPreferences::addLineEdit(
	const QString& label, const QString& helpText, QGridLayout* layout, int& row)
{
	QLineEdit* lineEdit = new QLineEdit(this);
	lineEdit->setObjectName(QStringLiteral("name"));
	lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

	addLabelAndWidget(label, lineEdit, layout, row);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return lineEdit;
}

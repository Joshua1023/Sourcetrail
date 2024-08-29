#include "utilityJava.h"
#include "utilityApp.h"
#include "utilityGradle.h"

#include "Application.h"
#include "ApplicationSettings.h"
#include "DialogView.h"
#include "JavaEnvironment.h"
#include "JavaEnvironmentFactory.h"
#include "MessageStatus.h"
#include "ResourcePaths.h"
#include "ScopedFunctor.h"
#include "TextAccess.h"
#include "logging.h"
#include "utilityString.h"

namespace
{

constexpr const char *getClassPathSeparator()
{
	if constexpr (utility::Platform::isWindows())
		return ";";
	else
		return ":";
}

}

namespace utility
{
std::vector<std::wstring> getRequiredJarNames()
{
	return {
		L"java-indexer.jar"
	};
}

std::string prepareJavaEnvironment()
{
	return prepareJavaEnvironment(ResourcePaths::getJavaDirectoryPath());
}

std::string prepareJavaEnvironment(const FilePath &javaDirectoryPath)
{
	std::string errorString;

	if (!JavaEnvironmentFactory::getInstance())
	{
		std::string classPath;
		{
			const std::vector<std::wstring> jarNames = getRequiredJarNames();
			for (size_t i = 0; i < jarNames.size(); i++)
			{
				if (i != 0)
				{
					classPath += getClassPathSeparator();
				}
				classPath += javaDirectoryPath.getConcatenated(L"lib/" + jarNames[i]).str();
			}
		}
		JavaEnvironmentFactory::createInstance(classPath, &errorString);
	}
	return errorString;
}

bool prepareJavaEnvironmentAndDisplayOccurringErrors()
{
	const std::wstring errorString = utility::decodeFromUtf8(utility::prepareJavaEnvironment());

	if (!errorString.empty())
	{
		LOG_ERROR(errorString);
		MessageStatus(errorString, true, false).dispatch();
	}

	if (!JavaEnvironmentFactory::getInstance())
	{
		std::wstring dialogMessage =
			L"Sourcetrail 无法在此机器上初始化 Java 环境。\n"
			"请确保在设置中提供正确的 Java 路径。";

		if (!errorString.empty())
		{
			dialogMessage += L"\n\nError: " + errorString;
		}

		MessageStatus(dialogMessage, true, false).dispatch();

		Application::getInstance()->handleDialog(dialogMessage);
		return false;
	}
	return true;
}

std::set<FilePath> fetchRootDirectories(const std::set<FilePath>& sourceFilePaths)
{
	std::shared_ptr<DialogView> dialogView = Application::getInstance()->getDialogView(
		DialogView::UseCase::PROJECT_SETUP);
	dialogView->showUnknownProgressDialog(L"项目准备中", L"正在收集根目录");

	ScopedFunctor dialogHider([&dialogView]() { dialogView->hideUnknownProgressDialog(); });

	std::set<FilePath> rootDirectories;

	std::shared_ptr<JavaEnvironment> javaEnvironment =
		JavaEnvironmentFactory::getInstance()->createEnvironment();
	for (const FilePath& filePath: sourceFilePaths)
	{
		std::shared_ptr<TextAccess> textAccess = TextAccess::createFromFile(filePath);

		std::string packageName;
		javaEnvironment->callStaticStringMethod(
			"com/sourcetrail/JavaIndexer", "getPackageName", packageName, textAccess->getText());

		if (packageName.empty())
		{
			continue;
		}

		FilePath rootPath = filePath.getParentDirectory();
		bool success = true;

		const std::vector<std::string> packageNameParts = utility::splitToVector(packageName, ".");
		for (std::vector<std::string>::const_reverse_iterator it = packageNameParts.rbegin();
			 it != packageNameParts.rend();
			 it++)
		{
			if (rootPath.fileName() != utility::decodeFromUtf8(*it))
			{
				success = false;
				break;
			}
			rootPath = rootPath.getParentDirectory();
		}

		if (success)
		{
			rootDirectories.insert(rootPath);
		}
	}

	return rootDirectories;
}

std::vector<FilePath> getClassPath(
	const std::vector<FilePath>& classpathItems,
	bool useJreSystemLibrary,
	const std::set<FilePath>& sourceFilePaths)
{
	std::vector<FilePath> classPath;

	for (const FilePath& classpath: classpathItems)
	{
		if (classpath.exists())
		{
			LOG_INFO(L"Adding path to classpath: " + classpath.wstr());
			classPath.push_back(classpath);
		}
	}

	if (useJreSystemLibrary)
	{
		for (const FilePath& systemLibraryPath:
			 ApplicationSettings::getInstance()->getJreSystemLibraryPathsExpanded())
		{
			LOG_INFO(L"Adding JRE system library path to classpath: " + systemLibraryPath.wstr());
			classPath.push_back(systemLibraryPath);
		}
	}

	for (const FilePath& rootDirectory: utility::fetchRootDirectories(sourceFilePaths))
	{
		if (rootDirectory.exists())
		{
			LOG_INFO(L"Adding root directory to classpath: " + rootDirectory.wstr());
			classPath.push_back(rootDirectory);
		}
	}

	return classPath;
}

void setJavaHomeVariableIfNotExists()
{
	if (getenv("JAVA_HOME") == nullptr)
	{
		const FilePath javaPath = ApplicationSettings::getInstance()->getJavaPath();
		const FilePath javaHomePath =
			javaPath.getParentDirectory().getParentDirectory().getParentDirectory();

		LOG_WARNING(
			"Environment variable \"JAVA_HOME\" not found on system. Setting value to \"" +
			javaHomePath.str() + "\" for this process.");
		putenv(const_cast<char*>(("JAVA_HOME=" + javaHomePath.str()).c_str()));
	}
}

}

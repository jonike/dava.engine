#include "CommandLine/SceneSaverTool.h"
#include "CommandLine/Private/REConsoleModuleTestUtils.h"

#include "Base/BaseTypes.h"


#include "Testing/TArcUnitTests.h"

namespace SSTestDetail
{
const DAVA::String projectStr = "~doc:/Test/SceneSaverTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
const DAVA::String newProjectStr = "~doc:/Test/SceneSaverTool_new/";
const DAVA::String newScenePathnameStr = newProjectStr + "DataSource/3d/Scene/testScene.sc2";
}

DAVA_TARC_TESTCLASS(SceneSaverToolTest)
{
    void EnumerateDescriptors(const DAVA::FilePath& folder, DAVA::List<DAVA::FilePath>& pathes)
    {
        using namespace DAVA;

        ScopedPtr<FileList> fileList(new FileList(folder));
        for (uint32 i = 0; i < fileList->GetCount(); ++i)
        {
            const FilePath& pathname = fileList->GetPathname(i);
            if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
            {
                EnumerateDescriptors(pathname, pathes);
            }
            else if (pathname.IsEqualToExtension(".tex"))
            {
                pathes.push_back(pathname);
            }
        }
    }

    bool CreateDummyCompressedFiles(const DAVA::FilePath& folder)
    {
        using namespace DAVA;

        List<FilePath> pathes;
        EnumerateDescriptors(folder, pathes);

        for (const FilePath& pathname : pathes)
        {
            std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(pathname));
            if (descriptor)
            {
                for (int32 gpu = 0; gpu < eGPUFamily::GPU_DEVICE_COUNT; ++gpu)
                {
                    FilePath compressedPath = descriptor->CreateMultiMipPathnameForGPU(static_cast<eGPUFamily>(gpu));
                    ScopedPtr<File> dummyFile(File::Create(compressedPath, File::CREATE | File::WRITE));
                }
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    DAVA_TEST (SaveSceneTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::TextureLoadingGuard guard = REConsoleModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SSTestDetail::scenePathnameStr);

        {
            REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::newProjectStr);
            FilePath dataSourcePath = SSTestDetail::projectStr + "DataSource/3d/";
            FilePath newDataSourcePath = SSTestDetail::newProjectStr + "DataSource/3d/";

            TEST_VERIFY(CreateDummyCompressedFiles(dataSourcePath));

            Vector<String> cmdLine =
            {
              "ResourceEditor",
              "-scenesaver",
              "-save",
              "-indir",
              dataSourcePath.GetAbsolutePathname(),
              "-outdir",
              newDataSourcePath.GetAbsolutePathname(),
              "-processfile",
              FilePath(SSTestDetail::scenePathnameStr).GetRelativePathname(dataSourcePath),
              "-copyconverted"
            };

            std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<SceneSaverTool>(cmdLine);
            REConsoleModuleTestUtils::ExecuteModule(tool.get());

            TEST_VERIFY(FileSystem::Instance()->Exists(SSTestDetail::newScenePathnameStr));

            REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::newProjectStr);
        }

        REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::projectStr);
    }

    DAVA_TEST (ResaveSceneTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::TextureLoadingGuard guard = REConsoleModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SSTestDetail::scenePathnameStr);

        FilePath dataSourcePath = SSTestDetail::projectStr + "DataSource/3d/";

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-scenesaver",
          "-resave",
          "-indir",
          dataSourcePath.GetAbsolutePathname(),
          "-processfile",
          FilePath(SSTestDetail::scenePathnameStr).GetRelativePathname(dataSourcePath),
        };

        std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<SceneSaverTool>(cmdLine);
        REConsoleModuleTestUtils::ExecuteModule(tool.get());

        TEST_VERIFY(FileSystem::Instance()->Exists(SSTestDetail::scenePathnameStr));

        REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::projectStr);
    }

    DAVA_TEST (ResaveYamlTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::TextureLoadingGuard guard = REConsoleModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SSTestDetail::scenePathnameStr);

        FilePath dataSourcePath = SSTestDetail::projectStr + "DataSource/3d/";

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-scenesaver",
          "-resave",
          "-yaml",
          "-indir",
          FilePath(SSTestDetail::projectStr).GetAbsolutePathname() + "Data/",
        };

        std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<SceneSaverTool>(cmdLine);
        REConsoleModuleTestUtils::ExecuteModule(tool.get());

        TEST_VERIFY(FileSystem::Instance()->Exists(SSTestDetail::projectStr + "Data/quality.yaml"));

        REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::projectStr);
    }

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("SceneSaverTool.cpp")
    END_FILES_COVERED_BY_TESTS();
};

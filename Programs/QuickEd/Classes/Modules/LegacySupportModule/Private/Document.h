#pragma once

#include <TArc/DataProcessing/DataContext.h>

#include <QString>

struct WidgetContext
{
    virtual ~WidgetContext() = 0;
};

inline WidgetContext::~WidgetContext()
{
}

namespace DAVA
{
class FilePath;
class CommandStack;
namespace TArc
{
class ContextAccessor;
}
}

class PackageNode;
class QtModelPackageCommandExecutor;

class Document
{
public:
    explicit Document(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::DataContext::ContextID contextId);
    ~Document();

    const DAVA::FilePath& GetPackageFilePath() const;
    PackageNode* GetPackage() const;
    QtModelPackageCommandExecutor* GetCommandExecutor() const;

    WidgetContext* GetContext(void* requester) const;
    void SetContext(void* requester, WidgetContext* widgetContext);

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    const DAVA::TArc::DataContext::ContextID contextId;

    std::unique_ptr<QtModelPackageCommandExecutor> commandExecutor;
    DAVA::UnorderedMap<void*, WidgetContext*> contexts;

    //store package to check that document is reloaded or not
    //TODO: remove this when PackageWidget will be separate module
    PackageNode* package = nullptr;
};

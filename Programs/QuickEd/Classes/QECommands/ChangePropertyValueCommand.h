#pragma once

#include "QECommands/Private/QEPackageCommand.h"

#include <FileSystem/VariantType.h>

class ControlNode;
class AbstractProperty;

class ChangePropertyValueCommand : public QEPackageCommand
{
public:
    ChangePropertyValueCommand(PackageNode* package);
    ChangePropertyValueCommand(PackageNode* package, ControlNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);

    void AddNodePropertyValue(ControlNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);

    void Redo() override;
    void Undo() override;

    void ApplyProperty(ControlNode* node, AbstractProperty* property, const DAVA::VariantType& value);

    bool MergeWith(const DAVA::Command* command) override;

private:
    struct Item
    {
        Item(ControlNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);
        DAVA::RefPtr<ControlNode> node;
        DAVA::RefPtr<AbstractProperty> property;
        DAVA::VariantType newValue;
        DAVA::VariantType oldValue;
    };
    DAVA::Vector<Item> items;
};

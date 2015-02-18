#ifndef __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
#define __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__

#include "PackageBaseNode.h"

#include "PackageControlsNode.h"

class PackageSerializer;

class ImportedPackagesNode : public PackageBaseNode
{
public:
    ImportedPackagesNode(PackageBaseNode *parent);
    virtual ~ImportedPackagesNode();

    void Add(PackageControlsNode *node);
    void InsertBelow(PackageControlsNode *node, const PackageControlsNode *belowThis);
    void Remove(PackageControlsNode *node);
    virtual int GetCount() const override;
    virtual PackageControlsNode *Get(int index) const override;
    
    virtual DAVA::String GetName() const;
    PackageControlsNode *FindPackageControlsNodeByName(const DAVA::String &name) const;

    virtual int GetFlags() const override;
    
    void Serialize(PackageSerializer *serializer) const;
    void Serialize(PackageSerializer *serializer, const DAVA::Set<PackageRef*> &packageRefs) const;
    
private:
    DAVA::Vector<PackageControlsNode*> packageControlsNode;
};



#endif //__UI_EDITOR_IMPORTED_PACKAGES_NODE_H__

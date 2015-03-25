#include "WidgetModel.h"

#include <QMetaObject>

#include "WidgetItem.h"


WidgetModel::WidgetModel( QWidget *w )
    : AbstractWidgetModel( w )
    , root( WidgetItem::create( w ) )
{
    root->rebuildChildren();
}

WidgetModel::~WidgetModel()
{
}

QWidget* WidgetModel::widgetFromIndex( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return nullptr;

    auto item = static_cast<WidgetItem *>( index.internalPointer() );

    return item->widget;
}

int WidgetModel::rowCount( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return 1;

    auto item = static_cast<WidgetItem *>( parent.internalPointer() );
    Q_ASSERT( item != nullptr );

    return item->children.size();
}

QModelIndex WidgetModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    if ( !parent.isValid() )
    {
        if ( row != 0 || column >= COLUMN_COUNT )
            return QModelIndex();

        return createIndex( 0, column, root.data() );
    }

    auto p = static_cast<WidgetItem *>( parent.internalPointer() );
    auto item = p->children.at( row );
    Q_ASSERT( item );

    return createIndex( row, column, item.data() );
}

QModelIndex WidgetModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    auto item = static_cast<WidgetItem *>( index.internalPointer() );
    auto parentItem = item->parentItem;
    if ( parentItem.isNull() )
        return QModelIndex();
    
    auto parentOfParentItem = parentItem->parentItem;
    if ( parentOfParentItem.isNull() )
        return createIndex( 0, 0, root.data() );

    // Getting row for parent
    auto row = -1;
    for ( auto i = 0; i < parentOfParentItem->children.size(); row++ )
    {
        if ( parentOfParentItem->children.at( i ) == parentItem )
        {
            row = i;
            break;
        }
    }
    Q_ASSERT( row >= 0 );

    return createIndex( row, 0, parentItem.data() );
}

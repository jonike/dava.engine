#include "ColorWidget.h"
#include "ui_ColorWidget.h"

#include "IColorEditor.h"
#include "HSVPaletteWidget.h"


ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ColorWidget())
{
    ui->setupUi(this);
    
    // TEST
    {
        ui->test_01->setColorRange( QColor( 255, 0, 0 ), QColor( 0, 0, 0, 0 ) );
        ui->test_01->setDimensions( true, false );
    }
    {
        ui->test_02->setColorRange( QColor( 0, 255, 0 ), QColor( 0, 0, 255 ) );
        ui->test_02->setDimensions( true, false );
        ui->test_02->setBgPadding( 20, 20, 20, 20 );
    }

    AddPalette( "HSV", new HSVPaletteWidget() );

    connect( ui->paletteCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( onPaletteType() ) );
}

ColorWidget::~ColorWidget()
{
}

void ColorWidget::AddPalette( QString const& name, IColorEditor* _pal )
{
    QWidget *pal = dynamic_cast<QWidget *>( _pal );
    Q_ASSERT( pal );

    ui->paletteCombo->addItem( name, name );
    ui->paletteStack->addWidget( pal );
    paletteMap[name] = pal;
}

void ColorWidget::onPaletteType()
{
    const int idx = ui->paletteCombo->currentIndex();
    //if ( idx < 0 )
    //    return;

    const QString& key = ui->paletteCombo->itemData( idx ).toString();
    ui->paletteStack->setCurrentWidget( paletteMap[key] );
}

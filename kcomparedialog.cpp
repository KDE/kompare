#include <kdialogbase.h>
#include <kurlrequester.h>
#include <kurlcombobox.h>
#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qgroupbox.h>

#include "kcomparedialog.h"

KCompareDialog::KCompareDialog( const KURL* sourceURL, const KURL* destinationURL, QWidget *parent, const char *name )
    :   KDialogBase( Plain, i18n("Compare Files"), Ok|Cancel, Ok, parent, name )
{
	QVBoxLayout* topLayout = new QVBoxLayout( plainPage(), 0,
	          spacingHint() );

	QGroupBox* sourceGB = new QGroupBox( i18n( "Source:" ), plainPage() );
	sourceGB->setColumnLayout(0, Qt::Vertical );
	sourceGB->layout()->setSpacing( 0 );
	sourceGB->layout()->setMargin( 0 );
	QHBoxLayout* sourceGBLayout = new QHBoxLayout( sourceGB->layout() );
	sourceGBLayout->setAlignment( Qt::AlignVCenter );
	sourceGBLayout->setSpacing( 6 );
	sourceGBLayout->setMargin( 11 );


	KConfig* config = kapp->config();
	config->setGroup( "Recent Files" );

	m_sourceURLComboBox = new KURLComboBox( KURLComboBox::Files, true );
	m_sourceURLComboBox->setURLs( config->readListEntry( "Recent Sources" ) );
	if( sourceURL ) {
		m_sourceURLComboBox->setURL( *sourceURL );
	}

	m_sourceURLRequester = new KURLRequester( m_sourceURLComboBox, sourceGB );
	m_sourceURLRequester->setFocus();
	sourceGBLayout->addWidget( m_sourceURLRequester );
	topLayout->addWidget( sourceGB );

	QGroupBox* destinationGB = new QGroupBox( i18n( "Destination:" ), plainPage() );
	destinationGB->setColumnLayout(0, Qt::Vertical );
	destinationGB->layout()->setSpacing( 0 );
	destinationGB->layout()->setMargin( 0 );
	QHBoxLayout* destinationGBLayout = new QHBoxLayout( destinationGB->layout() );
	destinationGBLayout->setAlignment( Qt::AlignVCenter );
	destinationGBLayout->setSpacing( 6 );
	destinationGBLayout->setMargin( 11 );

	m_destinationURLComboBox = new KURLComboBox( KURLComboBox::Files, true );
	m_destinationURLComboBox->setURLs( config->readListEntry( "Recent Destinations" ) );
	if( destinationURL ) {
		m_destinationURLComboBox->setURL( *destinationURL );
	}
	m_destinationURLRequester = new KURLRequester( m_destinationURLComboBox, destinationGB );

	destinationGBLayout->addWidget( m_destinationURLRequester );
	topLayout->addWidget( destinationGB );

	m_sourceURLRequester->setMinimumWidth( m_sourceURLRequester->sizeHint().width()*3 );
	setButtonOKText( i18n( "Compare" ), i18n( "Compare these files" ) );

	connect( m_sourceURLRequester, SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableCompare() ) );
	connect( m_destinationURLRequester, SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableCompare() ) );
	slotEnableCompare();
}

KCompareDialog::~KCompareDialog()
{
	KConfig* config = kapp->config();
	config->setGroup( "Recent Files" );
	config->writeEntry( "Recent Sources", m_sourceURLComboBox->urls() );
	config->writeEntry( "Recent Destinations", m_destinationURLComboBox->urls() );
}

void KCompareDialog::slotEnableCompare()
{
	enableButtonOK( !m_sourceURLRequester->url().isEmpty() &&
	                !m_destinationURLRequester->url().isEmpty() );
}

KURL KCompareDialog::getSourceURL() const
{
	if( result() == QDialog::Accepted )
		return m_sourceURLRequester->url();
	else
		return KURL();
}

KURL KCompareDialog::getDestinationURL() const
{
	if( result() == QDialog::Accepted )
		return m_destinationURLRequester->url();
	else
		return KURL();
}

#include "kcomparedialog.moc"

#ifndef KCOMPAREDIALOG_H
#define KCOMPAREDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KURLComboBox;
class KURLRequester;
class KFileDialog;

class KCompareDialog : public KDialogBase
{
	Q_OBJECT

public:
	KCompareDialog( const KURL* sourceURL = 0, const KURL* destURL = 0, QWidget* parent = 0, const char* name = 0 );
	~KCompareDialog();

	KURL getSourceURL() const;
	KURL getDestinationURL() const;

private slots:
	void slotEnableCompare();

private:
	KURLComboBox*      m_sourceURLComboBox;
	KURLComboBox*      m_destinationURLComboBox;
	KURLRequester*     m_sourceURLRequester;
	KURLRequester*     m_destinationURLRequester;
};

#endif

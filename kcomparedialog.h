#ifndef KCOMPAREDIALOG_H
#define KCOMPAREDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KURLCompletion;
class KURLRequester;
class KFileDialog;

class KCompareDialog : public KDialogBase
{
	Q_OBJECT

public:
	KCompareDialog( QWidget* parent = 0, const char* name = 0 );
	~KCompareDialog();

	KURL getSourceURL() const;
	KURL getDestinationURL() const;

private slots:
	void slotEnableCompare();

private:
	KURLRequester*     m_sourceURLRequester;
	KURLRequester*     m_destinationURLRequester;
};

#endif

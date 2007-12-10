#include "komparesaveoptionsbase.h"

#include <qvariant.h>
/*
 *  Constructs a KompareSaveOptionsBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
KompareSaveOptionsBase::KompareSaveOptionsBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : QWidget(parent, name, fl)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
KompareSaveOptionsBase::~KompareSaveOptionsBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KompareSaveOptionsBase::languageChange()
{
    retranslateUi(this);
}


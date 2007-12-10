#ifndef KOMPARESAVEOPTIONSBASE_H
#define KOMPARESAVEOPTIONSBASE_H

#include <qvariant.h>

#include "ui_komparesaveoptionsbase.h"

class KompareSaveOptionsBase : public QWidget, public Ui::KompareSaveOptionsBase
{
    Q_OBJECT

public:
    KompareSaveOptionsBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~KompareSaveOptionsBase();

protected slots:
    virtual void languageChange();

};

#endif // KOMPARESAVEOPTIONSBASE_H

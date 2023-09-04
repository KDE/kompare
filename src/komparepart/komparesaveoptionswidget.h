/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPARESAVEOPTIONSWIDGET_H
#define KOMPARESAVEOPTIONSWIDGET_H

#include <KompareDiff2/Global>

#include "komparesaveoptionsbase.h"

namespace KompareDiff2 {
class DiffSettings;
}
class QButtonGroup;

class KompareSaveOptionsWidget : public KompareSaveOptionsBase
{
    Q_OBJECT
public:
    KompareSaveOptionsWidget(const QString& source, const QString& destination,
                             KompareDiff2::DiffSettings* settings, QWidget* parent);
    ~KompareSaveOptionsWidget() override;

    void      saveOptions();
    QString   directory() const;

protected Q_SLOTS:
    void      updateCommandLine();

private:
    void      loadOptions();

    KompareDiff2::DiffSettings* m_settings;
    QString         m_source;
    QString         m_destination;
    QButtonGroup*   m_FormatBG;
};

#endif

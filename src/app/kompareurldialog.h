/*
    SPDX-FileCopyrightText: 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREURLDIALOG_H
#define KOMPAREURLDIALOG_H

#include <KPageDialog>

class FilesPage;
class FilesSettings;
class DiffPage;
class DiffSettings;
class ViewPage;
class ViewSettings;

/**
 * Definition of class KompareURLDialog.
 * @author Otto Bruggeman
 * @author John Firebaugh
 */
class KompareURLDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit KompareURLDialog(QWidget* parent = nullptr);
    ~KompareURLDialog() override;

    QUrl getFirstURL() const;
    QUrl getSecondURL() const;
    QString encoding() const;

    void setFirstGroupBoxTitle(const QString& title);
    void setSecondGroupBoxTitle(const QString& title);

    void setGroup(const QString& groupName);

    void setFirstURLRequesterMode(unsigned int mode);
    void setSecondURLRequesterMode(unsigned int mode);

public Q_SLOTS:
    void accept() override;
    void reject() override;

private Q_SLOTS:
    void slotEnableOk();
protected:
    void showEvent(QShowEvent* event) override;
private:
    FilesPage*     m_filesPage;
    FilesSettings* m_filesSettings;
    DiffPage*      m_diffPage;
    DiffSettings*  m_diffSettings;
    ViewPage*      m_viewPage;
    ViewSettings*  m_viewSettings;
};

#endif

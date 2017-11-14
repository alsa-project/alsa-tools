/*
 *  qlo10k1 - GUI frontend for ld10k1
 *
 *  Copyright (c) 2004 by Peter Zubaj
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
 
#ifndef NEWIODIALOG_H
#define NEWIODIALOG_H

#include "dlgs/newiodialogbase.h"
#include "strglobal.h"

class StrIOBase;

class NewIODlg : public NewIODialogBase
{
	Q_OBJECT
public:
	enum DlgType {In, Out, FX};
private:
	StrGlobal *global;
	DlgType dt;
	
	int num;
	QString name;
public:	
	NewIODlg(StrGlobal *glob, DlgType t);

	int init();
	
	StrIOBase *getNewIO();
public slots:
	void okClicked();
	void cancelClicked();
	
	void ioSelectionChanged(QListViewItem *item);
};

#endif // NEWIODIALOG_H


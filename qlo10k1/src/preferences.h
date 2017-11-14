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
 
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <qptrlist.h>

#include "dlgs/preferencesbase.h"
#include "strglobal.h"

class PreferencesDlg : public PreferencesBase
{
	Q_OBJECT
	StrCardGlobal *global;
	QPtrList <CardParam> cards;
	QString repDirSystem;
	QString repDirUser;
public:
	PreferencesDlg(StrCardGlobal *glob);
	void refreshListBox(void);
	void refreshListBox(int idx);
	
	void refreshRepListBox(void);
	void refreshRepListBox(int idx);
public slots:

	void cardHighlighted(int index);

	void okClicked();
	void cancelClicked();
	
	void updateClicked();
	void cardAddClicked();
	void cardDelClicked();
};

#endif // PREFERENCES_H

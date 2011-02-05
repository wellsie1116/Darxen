/* lvl3View.h
  
   Copyright (C) 2008 Kevin Wells <kevin@darxen.org>
    
   This file is part of darxen.
  
   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
#ifndef __LVL3VIEW_H__
#define __LVL3VIEW_H__

#include "common.h"
#include "Conversions.h"
#include "RadarViewer.h"
#include <gtkmm.h>
#include <libglademm.h>
#include <stdio.h>

class lvl3View : public Gtk::HBox
{
public:
	lvl3View(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~lvl3View();

	RadarViewer* get_radar_viewer();

private:
	Gtk::Label *lblRadarSite;
	Gtk::Label *lblProductCode;
	Gtk::Label *lblLatitude;
	Gtk::Label *lblLongitude;
	Gtk::Label *lblAltitude;
	Gtk::Label *lblVCP;
	Gtk::Label *lblScanTime;
	Gtk::Label *lblMaxDBZ;

	Gtk::CheckButton *chkSmoothing;
	RadarViewer *objRadar;

	void on_chkSmooth_toggled();
	void on_objRadar_frame_changed();
};

#endif
*/

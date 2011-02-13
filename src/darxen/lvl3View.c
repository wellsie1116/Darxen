/* lvl3View.c
  
   Copyright (C) 2011 Kevin Wells <kevin@darxen.org>
    
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
#include "lvl3View.h"

lvl3View::lvl3View(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::HBox(cobject),
  lblRadarSite(0),
  lblProductCode(0),
  lblLatitude(0),
  lblLongitude(0),
  lblAltitude(0),
  lblVCP(0),
  lblScanTime(0),
  lblMaxDBZ(0),
  chkSmoothing(0),
  objRadar(0)
{
	objRadar = new RadarViewer();

	Gtk::HBox *hboxRadarView = NULL;
	refGlade->get_widget("lblRadarSite", lblRadarSite);
	refGlade->get_widget("lblProductCode", lblProductCode);
	refGlade->get_widget("lblLatitude", lblLatitude);
	refGlade->get_widget("lblLongitude", lblLongitude);
	refGlade->get_widget("lblAltitude", lblAltitude);
	refGlade->get_widget("lblVCP", lblVCP);
	refGlade->get_widget("lblScanTime", lblScanTime);
	refGlade->get_widget("lblMaxDBZ", lblMaxDBZ);
	refGlade->get_widget("chkSmoothing", chkSmoothing);
	refGlade->get_widget("hboxRadarView", hboxRadarView);

	chkSmoothing->signal_toggled().connect(sigc::mem_fun(*this, &lvl3View::on_chkSmooth_toggled));
	objRadar->signal_frame_changed.connect(sigc::mem_fun(*this, &lvl3View::on_objRadar_frame_changed));
	hboxRadarView->pack_start(*objRadar, true, true, 0);

	objRadar->set_size_request(256, 256);

	show_all();
}

lvl3View::~lvl3View()
{
	
}

RadarViewer* lvl3View::get_radar_viewer()
{
	return objRadar;
}

void lvl3View::on_chkSmooth_toggled()
{
	objRadar->set_smoothing(chkSmoothing->get_active());
} //end on_chkSmooth_toggled

void lvl3View::on_objRadar_frame_changed()
{
	//update labels
	Products::Level3::Data objData = objRadar->get_current_frame_data();
	char chrRadarSite[5];
	char chrProductCode[4];
	int intH;
	int intM;
	int intS;
	char chrLatitude[40];
	char chrLongitude[40];
	char chrAltitude[40];
	char chrVCP[5];
	char chrScanTime[50];
	char chrMaxDBZ[20];
	objData.chrWmoHeader.get_radar_site(chrRadarSite);
	objData.chrWmoHeader.get_product_code(chrProductCode);
	sprintf(chrRadarSite, "%s", chrRadarSite);
	sprintf(chrProductCode, "%s", chrProductCode);
	Conversions::get_HMS(objData.objDescription.fltLat, intH, intM, intS);
	sprintf(chrLatitude, "%i deg %i' %i\" (%f)",intH, intM, intS, objData.objDescription.fltLat);
	Conversions::get_HMS(objData.objDescription.fltLon, intH, intM, intS);
	sprintf(chrLongitude, "%i deg %i' %i\" (%f)",intH, intM, intS, objData.objDescription.fltLon);
	sprintf(chrAltitude, "%.0f feet", objData.objDescription.fltAltitude * 1000.0f);
	sprintf(chrVCP, "%i", objData.objDescription.intVolCovPat);
	sprintf(chrScanTime, "Date: %i, Time: %i", objData.objDescription.intScanDate, objData.objDescription.intScanTime);
	sprintf(chrMaxDBZ, "%i decibels", objData.objDescription.intProdCodes[3]);
	lblRadarSite->set_text(chrRadarSite);
	lblProductCode->set_text(chrProductCode);
	lblLatitude->set_text(chrLatitude);
	lblLongitude->set_text(chrLongitude);
	lblAltitude->set_text(chrAltitude);
	lblVCP->set_text(chrVCP);
	lblScanTime->set_text(chrScanTime);
	lblMaxDBZ->set_text(chrMaxDBZ);
} //end on_objRadar_frame_changed()

*/


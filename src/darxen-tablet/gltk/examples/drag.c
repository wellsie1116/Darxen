/* drag.c
 *
 * Copyright (C) 2011 - Kevin Wells <kevin@darxen.org>
 *
 * This file is part of darxen
 *
 * darxen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * darxen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with darxen.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"

GltkWidget* create_dnd_list(const gchar* prefix, gboolean deletable);

GltkWidget* label_widget(GltkWidget* child, const char* message)
{
	GltkWidget* vbox = gltk_vbox_new(0);

	GltkWidget* hbox = gltk_hbox_new(0);

	gltk_box_append_widget(GLTK_BOX(vbox), gltk_label_new(message), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), child, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), hbox, FALSE, FALSE);

	return vbox;
}

GltkWidget*
create_composite_list_item(const char* caption, const char* btn)
{
	GltkWidget* hbox = gltk_hbox_new(0);

	gltk_box_append_widget(GLTK_BOX(hbox), gltk_label_new(caption), TRUE, TRUE);
	gltk_box_append_widget(GLTK_BOX(hbox), gltk_button_new(btn), TRUE, TRUE);

	return hbox;
}

GltkWidget* 
create_composite_list()
{
	GltkWidget* list = gltk_list_new();

	gltk_list_add_item(GLTK_LIST(list), create_composite_list_item("Item 1", "Btn 1"), NULL);
	gltk_list_add_item(GLTK_LIST(list), create_composite_list_item("Item 2", "Btn 2"), NULL);
	gltk_list_add_item(GLTK_LIST(list), create_composite_list_item("Item 3", "Btn 3"), NULL);

	return label_widget(list, "Any widget can be added to a list.\nThis includes composite widgets and\neven additional lists.");
}

typedef struct
{
	char prefix;
	int index;
} ItemInfo;

GltkWidget* listWidget1;
GltkWidget* listWidget2;

GltkWidget*
convert_tracks_list_item(GltkList* list, const gchar* prefix, const GltkListItem* item, GltkList* listOther)
{
	GltkWidget* vbox = gltk_vbox_new(0);
	{
		GltkWidget* label = gltk_label_new(GLTK_WIDGET(list) == listWidget1 ? "From B:" : "From A:");
		GltkWidget* hbox = gltk_hbox_new(0);
		{
			gltk_box_append_widget(GLTK_BOX(hbox), gltk_spacer_new(10, 0), FALSE, FALSE);
			gltk_box_append_widget(GLTK_BOX(hbox), item->widget, FALSE, FALSE);
		}
		gltk_box_append_widget(GLTK_BOX(vbox), label, FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(vbox), hbox, FALSE, FALSE);
	}
	return vbox;
}

GltkWidget*
create_tracks_list(gchar prefix, gboolean deletable)
{
	GltkWidget* list = gltk_list_new();
	g_object_set(list,	"target-type", "WidgetDrop", 
						"deletable", deletable, NULL);

	int i;
	ItemInfo* infos = g_new(ItemInfo, 5);
	for (i = 0; i < 5; i++)
	{
		infos[i].prefix = prefix;
		infos[i].index = i;
		gchar* label = g_strdup_printf("%c-%i", prefix, i);
		gltk_list_add_item(GLTK_LIST(list), gltk_label_new(label), infos + i);
		g_free(label);
	}

	return list;
}

void
bin_drop_item(GltkWidget* widget, const gchar* type, const GltkListItem* item)
{
	g_assert(!g_strcmp0(type, "WidgetDrop"));

	gltk_bin_set_widget(GLTK_BIN(widget), item->widget);
	ItemInfo* info = (ItemInfo*)item->data;

	GltkWidget* list = listWidget2;
	if (info->prefix == 'A')
		list = listWidget1;

	gchar* label = g_strdup_printf("%c-%i", info->prefix, info->index);
	gltk_list_add_item(GLTK_LIST(list), gltk_label_new(label), info);
	g_free(label);
}

GltkWidget*
create_tracks_test()
{
	GltkWidget* hbox = gltk_hbox_new(5);
	{
		listWidget1 = create_tracks_list('A', TRUE);
		listWidget2 = create_tracks_list('B', FALSE);
		g_signal_connect(G_OBJECT(listWidget1), "convert-dropped-item", G_CALLBACK(convert_tracks_list_item), listWidget2);
		g_signal_connect(G_OBJECT(listWidget2), "convert-dropped-item", G_CALLBACK(convert_tracks_list_item), listWidget1);

		const static gchar* lbl1 = "Drag items to the list\nto the right.\nItems will remember where\nthey came from";
		const static gchar* lbl2 = "Drag items to the list\nto the left.\nItems will remember where\nthey came from";
		const static gchar* lblGraveyard = "Drag items here\nto reset their history";

		GltkWidget* lGraveyard = gltk_label_new("Drag an item here");
		gltk_label_set_draw_border(GLTK_LABEL(lGraveyard), TRUE);
		GltkWidget* graveyard = gltk_bin_new(lGraveyard);
		g_object_set(graveyard, "target-type", "WidgetDrop", NULL);
		g_signal_connect(G_OBJECT(graveyard), "drop-item", G_CALLBACK(bin_drop_item), NULL);
		GltkSize size = {200, 200};
		gltk_widget_set_size_request(graveyard, size);

		gltk_box_append_widget(GLTK_BOX(hbox), label_widget(listWidget1, lbl1), FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(hbox), label_widget(listWidget2, lbl2), FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(hbox), label_widget(graveyard, lblGraveyard), FALSE, FALSE);
	}

	return hbox;
}

void
dnd_list_item_deleted(GltkList* list2, GltkListItem* item, GltkList* list1)
{
	char id;
	int i;
	sscanf(GLTK_LABEL(item->widget)->text, "Item %c-%i", &id, &i);
	if (id != 'A')
		return;

	gltk_list_add_item(GLTK_LIST(list1), item->widget, NULL);
}

GltkWidget*
create_dnd_list(const gchar* prefix, gboolean deletable)
{
	GltkWidget* list = gltk_list_new();
	g_object_set(list,	"target-type", "SiteList", 
						"deletable", deletable, NULL);

	int i;
	for (i = 0; i < 5; i++)
	{
		gchar* label = g_strdup_printf("Item %s-%i", prefix, i);
		gltk_list_add_item(GLTK_LIST(list), gltk_label_new(label), NULL);
	}

	//g_signal_connect(G_OBJECT(list), "drop-item", G_CALLBACK(list_drop_item), NULL);

	return list;
}

GltkWindow*
create_window()
{
	GltkScreen* screen = gltk_screen_new();
	GltkWindow* window = gltk_window_new();


	GltkWidget* vbox = gltk_vbox_new(5);
	{
		GltkWidget* hboxWidgets = gltk_hbox_new(0);
		{
			//gltk_box_append_widget(GLTK_BOX(hboxWidgets), create_label_list(), TRUE, FALSE);
			//gltk_box_append_widget(GLTK_BOX(hboxWidgets), create_button_list(), TRUE, FALSE);
			gltk_box_append_widget(GLTK_BOX(hboxWidgets), create_tracks_test(), TRUE, FALSE);
		}

		GltkWidget* hboxDND = gltk_hbox_new(0);
		{
			static const gchar* lbl1 = "Non deletable items,\ndraggable to the list\nto the right";
			static const gchar* lbl2 = "Deletable items,\ndraggable to the list\nto the left";
			GltkWidget* list1 = create_dnd_list("A", FALSE);
			GltkWidget* list2 = create_dnd_list("B", TRUE);
			gltk_box_append_widget(GLTK_BOX(hboxDND), label_widget(list1, lbl1), TRUE, FALSE);
			gltk_box_append_widget(GLTK_BOX(hboxDND), label_widget(list2, lbl2), TRUE, FALSE);
			g_signal_connect(G_OBJECT(list2), "item-deleted", G_CALLBACK(dnd_list_item_deleted), list1);
		}

		gltk_box_append_widget(GLTK_BOX(vbox), hboxWidgets, FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(vbox), hboxDND, FALSE, FALSE);
	}

	gltk_screen_set_root(screen, vbox);
	gltk_window_push_screen(window, screen);

	return window;
}


package me.kevinwells.darxen;

import java.util.Date;

import android.content.Context;
import android.content.SharedPreferences;

public class Prefs {
	
	private static final String PREFS_NAME = "prefs";
	private static final String PREF_UPDATE_TIME = "UPDATE_TIME";
	
	public static void unsetLastUpdateTime() {
		getPrefs().edit()
		.remove(PREF_UPDATE_TIME)
		.commit();
	}
	
	public static void setLastUpdateTime(Date time) {
		getPrefs().edit()
		.putLong(PREF_UPDATE_TIME, time.getTime())
		.commit();
	}
	
	public static Date getLastUpdateTime() {
		SharedPreferences prefs = getPrefs();
		if (!prefs.contains(PREF_UPDATE_TIME))
			return null;
		
		return new Date(getPrefs().getLong(PREF_UPDATE_TIME, 0));
	}
	
	private static SharedPreferences getPrefs() {
		return MyApplication.getInstance().getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
	}

}

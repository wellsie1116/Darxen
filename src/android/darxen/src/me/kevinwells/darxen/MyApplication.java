package me.kevinwells.darxen;

import android.app.Application;

public class MyApplication extends Application {
	
	private static MyApplication instance = null;
	
	public static MyApplication getInstance() {
		return instance;
	}

	@Override
	public void onCreate() {
		instance = this;
		super.onCreate();
	}
}

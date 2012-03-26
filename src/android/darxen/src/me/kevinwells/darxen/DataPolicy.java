package me.kevinwells.darxen;

import java.util.Calendar;
import java.util.Date;

public class DataPolicy {
	
	private static final int MIN_UPDATE_PERIOD = 60;
	
	public static boolean shouldUpdate(Date lastUpdate, Date now) {
		if (lastUpdate == null)
			return true;
		
		return addSeconds(lastUpdate, MIN_UPDATE_PERIOD).before(now);
	}
	
	private static Date addSeconds(Date time, int seconds) {
		Calendar cal = Calendar.getInstance();
		cal.setTime(time);
		cal.add(Calendar.SECOND, seconds);
		return cal.getTime();
	}

}

package me.kevinwells.darxen.data;

import java.io.IOException;
import java.io.InputStream;

public interface Parsable<T> {
	public T parse(InputStream stream) throws ParseException, IOException;	
}

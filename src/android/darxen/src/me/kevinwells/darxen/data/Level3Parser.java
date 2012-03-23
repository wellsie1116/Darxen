package me.kevinwells.darxen.data;

import java.io.IOException;
import java.io.InputStream;

public class Level3Parser {
	
	public Level3Parser() {
		
	}
	
	public DataFile parse(InputStream stream) throws ParseException, IOException {
		return DataFile.parse(new DataFileStream(stream));
	}

}

package me.kevinwells.darxen.data;

import java.io.IOException;

public class Description {
	
	public float lat;
	public float lon;
	
	public OpMode opmode;
	
	public SymbologyBlock symbologyBlock;

	public static Description parse(DataFileStream stream) throws ParseException, IOException {
		Description res = new Description();
		
		if (stream.readShort() != -1)
			throw new ParseMagicException("Description Block Header");
		
		res.lat = (float)stream.readInt() / 1000.0f;
		res.lon = (float)stream.readInt() / 1000.0f;
		stream.readShort();
		stream.readShort();
		short opmode = stream.readShort();
		switch (opmode) {
		case 0:
			res.opmode = OpMode.MAINTENANCE;
			break;
		case 1:
			res.opmode = OpMode.CLEAN_AIR;
			break;
		case 2:
			res.opmode = OpMode.PRECIPITATION;
			break;
		default:
			throw new ParseException("Invalid Operation Mode");	
		}
		stream.readShort();
		stream.readShort();
		stream.readShort();
		stream.readShort();
		stream.readInt();
		stream.readShort();
		stream.readInt();
		
		//prod specific codes
		stream.readShort();
		stream.readShort();
		
		stream.readShort();
		
		stream.readShort();
		
		for (int i = 0; i < 16; i++) {
			stream.readByte();
			stream.readByte();
		}
		
		for (int i = 0; i < 7; i++) {
			stream.readShort();
		}
		if (stream.readByte() > 1)
			throw new ParseMagicException("Description Block blank");
		stream.readByte();
		
		int symbOffset = stream.readInt();
		stream.readInt();
		stream.readInt();
		
		if (symbOffset > 0) {
			res.symbologyBlock = SymbologyBlock.parse(stream);
		}
		
		return res;
	}
	
	public enum OpMode {
		MAINTENANCE,
		CLEAN_AIR,
		PRECIPITATION
	}

}

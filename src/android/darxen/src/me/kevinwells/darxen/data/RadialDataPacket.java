package me.kevinwells.darxen.data;

import java.io.IOException;

public class RadialDataPacket extends Packet {

	public short firstRangeBin;
	public short rangeBinCount;
	public short centerSweepI;
	public short centerSweepJ;
	public float scaleFactor;
	public RadialPacket[] radials;
	
	public static RadialDataPacket parse(DataFileStream stream) throws ParseException, IOException {
		RadialDataPacket res = new RadialDataPacket();
		
		res.firstRangeBin = stream.readShort();
		res.rangeBinCount = stream.readShort();
		res.centerSweepI = stream.readShort();
		res.centerSweepJ = stream.readShort();
		res.scaleFactor = stream.readShort() / 1000.0f;
		
		short radialCount = stream.readShort();
		res.radials = new RadialPacket[radialCount];
		for (int i = 0; i < radialCount; i++) {
			res.radials[i] = res.parseRadialPacket(stream);
		}
		
		return res;
	}
	
	private RadialPacket parseRadialPacket(DataFileStream stream) throws ParseException, IOException {
		RadialPacket res = new RadialPacket();
		
		short radialRleCount = (short)(stream.readShort() * 2);
		res.start = stream.readShort() / 10.0f;
		res.delta = stream.readShort() / 10.0f;
		
		res.codes = new byte[rangeBinCount];
		int pos = 0;
		for (int j = 0; j < radialRleCount; j++) {
			int rle = stream.readByte() & 0xFF;
			int rleCount = ((int)rle) >> 4;
			rle &= 0x0F;
			for (int k = 0; k < rleCount; k++) {
				res.codes[k + pos] = (byte)rle;
			}
			pos += rleCount;
		}
		
		return res;
	}
	
}

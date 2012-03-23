package me.kevinwells.darxen.data;

import java.io.IOException;

public class SymbologyBlock {
	
	public short layerCount;
	public short[] packetCodes;
	public Packet[] packets;

	public static SymbologyBlock parse(DataFileStream stream) throws ParseException, IOException {
		SymbologyBlock res = new SymbologyBlock();
		
		if (stream.readShort() != -1 || stream.readShort() != 1)
			throw new ParseMagicException("Symbology Block Header");
		
		stream.readInt();
		res.layerCount = stream.readShort();
		
		res.packetCodes = new short[res.layerCount];
		res.packets = new Packet[res.layerCount];
		
		for (int i = 0; i < res.layerCount; i++) {
			if (stream.readShort() != -1)
				throw new ParseMagicException("Symbology Block Layer Divider");
			
			stream.readInt();
			res.packetCodes[i] = stream.readShort();
			
			switch (res.packetCodes[i]) {
			case (short)0xAF1F: /* Radial Data Packet */
				res.packets[i] = RadialDataPacket.parse(stream);
				break;
			default:
				throw new ParseMagicException("Symbology Block Packet Type");
			}
		}
		
		return res;
	}
	
}

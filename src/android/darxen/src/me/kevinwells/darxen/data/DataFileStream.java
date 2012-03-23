package me.kevinwells.darxen.data;

import java.io.IOException;
import java.io.InputStream;

public class DataFileStream {
	
	private InputStream s;
	
	public DataFileStream(InputStream stream) {
		s = stream;
	}
	
	public void skip(long n) throws ParseException, IOException {
		long l = s.skip(n);
		if (l != n)
			throw new ParseException("Invalid skip");
	}
	
	public void read(byte[] b) throws ParseException, IOException {
		long l = s.read(b);
		if (l != b.length)
			throw new ParseException("Invalid read");
	}
	
	public byte readByte() throws ParseException, IOException {
		int r = s.read();
		//TODO check range
		return (byte)r;
	}

	public int readInt() throws ParseException, IOException {
		byte[] b = new byte[4];
		read(b);
		
		return ((b[0] & 0xFF) << 24) |
				((b[1] & 0xFF) << 16) |
				((b[2] & 0xFF) << 8) |
				((b[3] & 0xFF));
	}

	public short readShort() throws ParseException, IOException {
		byte[] b = new byte[2];
		read(b);
		
		return (short)(
				(((int)b[0] & 0xFF) << 8) |
				(((int)b[1] & 0xFF)));
	}
	
}

package me.kevinwells.darxen.data;

import java.io.IOException;

public class DataFile {
	
	public byte[] header;
	
	public MessageHeader messageHeader;
	
	public Description description;

	public static DataFile parse(DataFileStream stream) throws ParseException, IOException {
		DataFile res = new DataFile();
		
		res.header = new byte[27];
		stream.read(res.header);
		
		res.messageHeader = MessageHeader.parse(stream);
		res.description = Description.parse(stream);
		
		return res;
	}

}

package me.kevinwells.darxen.shp;

public class Shapefile {
	
	private long hShp;
	
	private int entities;
	private int shapeType;

	public Shapefile() {
	}
	
	public native String open(String path);
	public native String close();
	public native ShapefileObject get(int i);
	
	public int getShapeCount() {
		return entities;
	}

	public int getType() {
		return shapeType;
	}
	
	static {
		System.loadLibrary("shp");
	}
}

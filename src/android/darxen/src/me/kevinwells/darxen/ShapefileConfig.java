package me.kevinwells.darxen;

public class ShapefileConfig {
	public int resShp;
	public int resShx;
	public int resDbf;
	
	public float lineWidth;
	public Color color;
	
	public ShapefileConfig(int resShp, int resDbf, int resShx, float lineWidth,
			Color color) {
		this.resShp = resShp;
		this.resDbf = resDbf;
		this.resShx = resShx;
		this.lineWidth = lineWidth;
		this.color = color;
	}
}

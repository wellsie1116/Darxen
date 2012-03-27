package me.kevinwells.darxen.shp;

public class ShapefileObject {
	
	public long hShape;
	
	//public int nSHPType;
	//public int nShapeId;
	public int nParts;
	public int[] panPartStart;
	public int[] panPartType;
	public int nVertices;
	public double[] padfX;
	public double[] padfY;
	public double[] padfZ;
	//public double[] padfM;
	public double dfXMin;
	public double dfYMin;
	//public double dfZMin;
	//public double dfMMin;
	public double dfXMax;
	public double dfYMax;
	//public double dfZMax;
	//public double dfMMax;
	//public int bMeasureIsUsed;

	public ShapefileObject(int nParts, int nVertices, double dfXMin,
			double dfYMin, double dfXMax, double dfYMax,
			long hShape) {
		this.nParts = nParts;
		this.nVertices = nVertices;
		this.dfXMin = dfXMin;
		this.dfYMin = dfYMin;
		this.dfXMax = dfXMax;
		this.dfYMax = dfYMax;
		this.hShape = hShape;
		init();
	}
	
	public native void init();
	public native void load();
	public native void close();
	
	private boolean doesIntersect(double r1, double r2, double s1, double s2) {
		// Positive intersection
		// A: |--------|            |--------|
		// B:     |--------|          |----|
				
		// Negative intersection
		// A: |--------|         
		// B:          |--------|
		return !((s1 <= r1) && (s2 <= r1)) ^ ((s1 >= r2) && (s2 >= r2));
	}
	
	private static final double DISPLAY_RADIUS = 4.0;
	
	public boolean isNear(double lat, double lon) {
		assert(dfXMin < dfXMax);
		assert(dfYMin < dfYMax);
		
		return doesIntersect(dfXMin, dfXMax, lon-DISPLAY_RADIUS, lon+DISPLAY_RADIUS) &&
				doesIntersect(dfYMin, dfYMax, lat-DISPLAY_RADIUS, lat+DISPLAY_RADIUS);
	}
	
	public ShapefilePoint getPoint(int i, ShapefilePoint pt) {
		if (pt == null)
			pt = new ShapefilePoint();
		pt.x = this.padfX[i];
		pt.y = this.padfY[i];
		pt.z = this.padfZ[i];
		return pt;
	}


}

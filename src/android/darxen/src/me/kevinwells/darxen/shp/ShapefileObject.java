package me.kevinwells.darxen.shp;

public class ShapefileObject {
	
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
	
	public ShapefileObject(int nParts, int[] panPartStart, int[] panPartType,
			int nVertices, double[] padfX, double[] padfY, double[] padfZ,
			double dfXMin, double dfYMin, double dfXMax, double dfYMax) {
		this.nParts = nParts;
		this.panPartStart = panPartStart;
		this.panPartType = panPartType;
		this.nVertices = nVertices;
		this.padfX = padfX;
		this.padfY = padfY;
		this.padfZ = padfZ;
		this.dfXMin = dfXMin;
		this.dfYMin = dfYMin;
		this.dfXMax = dfXMax;
		this.dfYMax = dfYMax;
	}
	
	public boolean contains(double lat, double lon) {
		return (dfYMin < lat && dfYMax > lat &&
				dfXMin < lon && dfXMax > lon);
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

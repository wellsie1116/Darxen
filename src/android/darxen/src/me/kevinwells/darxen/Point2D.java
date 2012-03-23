package me.kevinwells.darxen;

public class Point2D {

	public double x;
	public double y;

	public Point2D() {
	}

	public Point2D(double x, double y) {
		this.x = x;
		this.y = y;
	}

	public Point2D add(double x, double y) {
		this.x += x;
		this.y += y;
		return this;
	}

	public Point2D divide(double value) {
		this.x /= value;
		this.y /= value;
		return this;
	}

	public Point2D subtract(Point2D pt, Point2D res) {
		if (res == null)
			res = new Point2D();
		res.x = x - pt.x;
		res.y = y - pt.y;
		return res;
	}
	public Point2D subtract(Point2D pt) {
		return this.subtract(pt, this);
	}
	
	public double distanceTo(double x, double y) {
		return Math.sqrt(Math.pow(x - this.x, 2) + Math.pow(y - this.y, 2));
	}
}

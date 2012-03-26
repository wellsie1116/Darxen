package me.kevinwells.darxen;

public class LatLon {
	
	public double lat;
	public double lon;

	public LatLon() {
	}
	
	public LatLon(double lat, double lon) {
		this.lat = lat;
		this.lon = lon;
	}
	
	public double distanceTo(LatLon o) {
		final double R = 6371.0; //km
		double dLat = Math.toRadians(o.lat - lat);
		double dLon = Math.toRadians(o.lon - lon);
		double lat1 = Math.toRadians(lat);
		double lat2 = Math.toRadians(o.lat);
		
		double a = Math.sin(dLat/2) * Math.sin(dLat/2) +
				Math.sin(dLon/2) * Math.sin(dLon/2) * Math.cos(lat1) * Math.cos(lat2);
		double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
		return R * c;
	}
	
	public Point2D project(LatLon center, Point2D res) {
		if (res == null)
			res = new Point2D();
		
		Point3D geoCenter = projectToGeocentric(center);
		Point3D geoOffset = projectToGeocentric(this);
		
		Point3D local = projectToLocal(center, geoCenter, geoOffset);
		
		res.x = local.x;
		res.y = local.y;
		return res;
	}
	
	private static Point3D projectToGeocentric(LatLon pt) {
		final double a = 6378137.0;
		final double b = 6356752.314140;
		
		double lat = Math.toRadians(pt.lat);
		double lon = Math.toRadians(pt.lon);

		double ae = Math.acos(b / a);

		double height = 0.0;

		double inner = Math.sin(lat) * Math.sin(ae);

		double N = a / Math.sqrt(1.0 - inner * inner);

		double x = (N + height) * Math.cos(lat) * Math.cos(lon);
		double y = (N + height) * Math.cos(lat) * Math.sin(lon);
		double z = (Math.pow(Math.cos(ae), 2.0) * N + height) * Math.sin(lat);

		//to km
		x /= 1000.0;
		y /= 1000.0;
		z /= 1000.0;

		return new Point3D(x, y, z);
	}
	
	private Point3D projectToLocal(LatLon center, Point3D geoCenter, Point3D geoOffset) {
		double offsetX = geoOffset.x - geoCenter.x;
		double offsetY = geoOffset.y - geoCenter.y;
		double offsetZ = geoOffset.z - geoCenter.z;

		double geodeticCenterLat = Math.toRadians(center.lat);
		double geodeticCenterLon = Math.toRadians(center.lon);
		
		double resX = 	offsetX * -Math.sin(geodeticCenterLon) +
						offsetY * Math.cos(geodeticCenterLon);
		double resY = 	offsetX * -Math.sin(geodeticCenterLat) * Math.cos(geodeticCenterLon) +
						offsetY * -Math.sin(geodeticCenterLat) * Math.sin(geodeticCenterLon) +
						offsetZ * Math.cos(geodeticCenterLat);
		double resZ = 	offsetX * Math.cos(geodeticCenterLat) * Math.cos(geodeticCenterLon) +
						offsetY * Math.cos(geodeticCenterLat) * Math.sin(geodeticCenterLon) +
						offsetZ * Math.sin(geodeticCenterLat);
		
		return new Point3D(resX, resY, resZ);
	}

}

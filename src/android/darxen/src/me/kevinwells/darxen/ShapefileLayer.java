package me.kevinwells.darxen;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.opengles.GL10;

import me.kevinwells.darxen.shp.Shapefile;
import me.kevinwells.darxen.shp.ShapefileObject;
import me.kevinwells.darxen.shp.ShapefilePoint;

public class ShapefileLayer implements Renderable {
	
	private LatLon mCenter;
	private List<Renderable> mBufs;
	private ShapefileConfig mConfig;
	
	public ShapefileLayer(LatLon center, Shapefile shp, ShapefileConfig config) {
		mCenter = center;
		mBufs = new ArrayList<Renderable>(); 
		mConfig = config;
		
		for (int i = 0; i < shp.getShapeCount(); i++) {
			ShapefileObject obj = shp.get(i);
			
			if (!obj.isNear(center.lat, center.lon)) {
				obj.close();
				continue;
			} else {
				obj.load();
			}

			for (int j = 0; j < obj.nParts; j++) {
				int start = obj.panPartStart[j];
				int end = start;
				for (int k = start; k < obj.nVertices; k++) {
					end = k+1;
					if (j < obj.nParts-1 && k+1 == obj.panPartStart[j+1])
						break;
				}
				mBufs.add(generateRenderable(obj, start, end));
			}
		}
	}
	private LatLon latLon = new LatLon();
	private ShapefilePoint shapePt = new ShapefilePoint();
	private Point2D p2 = new Point2D();
	private Renderable generateRenderable(ShapefileObject obj, int start, int end) {		
		ByteBuffer vbb = ByteBuffer.allocateDirect((end-start) * 2 * 4);
		vbb.order(ByteOrder.nativeOrder());
		FloatBuffer buf = vbb.asFloatBuffer();
		
		for (int i = start; i < end; i++) {
			obj.getPoint(i, shapePt);
			latLon.lat = shapePt.y;
			latLon.lon = shapePt.x;
			p2 = latLon.project(mCenter, p2);
			buf.put((float)p2.x);
			buf.put((float)p2.y);
		}
		buf.position(0);
		
		return new SimpleRenderable(buf, (end-start), mConfig.lineWidth, mConfig.color);
	}

	@Override
	public void render(GL10 gl) {
		for (Renderable renderable : mBufs) {
			renderable.render(gl);
		}
	}

}

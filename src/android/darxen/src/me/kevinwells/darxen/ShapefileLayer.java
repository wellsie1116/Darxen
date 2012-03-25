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
	
	public ShapefileLayer(LatLon center, Shapefile shp) {
		mCenter = center;
		mBufs = new ArrayList<Renderable>(); 
		
		for (int i = 0; i < shp.getShapeCount(); i++) {
			ShapefileObject obj = shp.get(i);
			
			if (!obj.contains(center.lat, center.lon))
				continue;
			
			for (int j = 0; j < obj.nParts; j++) {
				int start = j;
				int end = start;
				for (int k = obj.panPartStart[j]; k < obj.nVertices; k++) {
					end = k+1;
					if (j < obj.nParts-1 && k+1 == obj.panPartStart[j+1])
						break;
				}
				mBufs.add(generateRenderable(obj, start, end));
			}
		}
	}
	
	private Renderable generateRenderable(ShapefileObject obj, int start, int end) {
		LatLon pt = new LatLon();
		ShapefilePoint sPt = new ShapefilePoint();
		
		ByteBuffer vbb = ByteBuffer.allocateDirect((end-start) * 2 * 4);
		vbb.order(ByteOrder.nativeOrder());
		FloatBuffer buf = vbb.asFloatBuffer();
		
		for (int i = start; i < end; i++) {
			obj.getPoint(i, sPt);
			pt.lat = sPt.y;
			pt.lon = sPt.x;
			Point2D xy = pt.project(mCenter);
			buf.put((float)xy.x);
			buf.put((float)xy.y);
		}
		buf.position(0);
		
		return new SimpleRenderable(buf, (end-start), new Color(1.0f, 1.0f, 1.0f));
	}

	@Override
	public void render(GL10 gl) {
		for (Renderable renderable : mBufs) {
			renderable.render(gl);
		}
	}

}

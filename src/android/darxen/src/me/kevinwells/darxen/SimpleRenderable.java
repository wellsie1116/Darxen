package me.kevinwells.darxen;

import java.nio.FloatBuffer;

import javax.microedition.khronos.opengles.GL10;

public class SimpleRenderable implements Renderable {
	
	private FloatBuffer mBuffer;
	private int mCount;
	private float mLineWidth;
	private Color mColor;

	public SimpleRenderable(FloatBuffer buffer, int count, float lineWidth, Color color) {
		mBuffer = buffer;
		mCount = count;
		mLineWidth = lineWidth;
		mColor = color;
	}

	@Override
	public void render(GL10 gl) {
		gl.glColor4f(mColor.r, mColor.g, mColor.b, 1.0f);
		gl.glVertexPointer(2, GL10.GL_FLOAT, 0, mBuffer);
		gl.glLineWidth(mLineWidth);
		gl.glDrawArrays(GL10.GL_LINE_STRIP, 0, mCount);
	}
	
}

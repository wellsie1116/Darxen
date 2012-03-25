package me.kevinwells.darxen;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.SocketException;

import me.kevinwells.darxen.data.DataFile;
import me.kevinwells.darxen.data.Level3Parser;
import me.kevinwells.darxen.data.ParseException;
import me.kevinwells.darxen.shp.Shapefile;

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPReply;

import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.actionbarsherlock.app.SherlockActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;

public class MapActivity extends SherlockActivity {
	
	private RadarView mRadarView;
	
    private LocationManager locationManager;
    private LocationListener locationListener;
    
    private boolean mLayersLoaded;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mRadarView = new RadarView(this);
        ((FrameLayout)findViewById(R.id.container)).addView(mRadarView);
        
        update();
    }
    
    @Override
	public boolean onCreateOptionsMenu(Menu menu) {
    	getSupportMenuInflater().inflate(R.menu.map, menu);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.refresh:
			update();
			break;
		default:
			return false;
		}
		return true;
	}
	
	private void update() {
        byte[] data;
        try {
        	data = getData();
        } catch (SocketException e) {
			Log.e(C.TAG, "Failed to download radar imagery", e);
			finish();
			return;
		} catch (IOException e) {
			Log.e(C.TAG, "Failed to download radar imagery", e);
			finish();
			return;
		}
        
        Level3Parser parser = new Level3Parser();
        DataFile file;
        try {
			file = parser.parse(new ByteArrayInputStream(data));
		} catch (ParseException e) {
			Log.e(C.TAG, "Failed to parse radar imagery", e);
			finish();
			return;
		} catch (IOException e) {
			Log.e(C.TAG, "Failed to parse radar imagery", e);
			finish();
			return;
		}
        
        TextView title = (TextView)findViewById(R.id.title);
        title.setText(new String(file.header).replace('\n', ' '));
        
        if (!mLayersLoaded) {
        	new LoadShapefiles(new LatLon(file.description.lat, file.description.lon)).execute();
        }
        
        mRadarView.setData(file);
	}

	private byte[] getData() throws SocketException, IOException {
    	ByteArrayOutputStream fout = new ByteArrayOutputStream();
        
    	FTPClient ftpClient = new FTPClient();
		ftpClient.connect("tgftp.nws.noaa.gov", 21);
		if (!FTPReply.isPositiveCompletion(ftpClient.getReplyCode()))
			throw new IOException("Failed to connect");
		ftpClient.login("anonymous", "darxen");
		
		ftpClient.changeWorkingDirectory("SL.us008001/DF.of/DC.radar/DS.p19r0/SI.kind");
		ftpClient.setFileType(FTP.BINARY_FILE_TYPE);
		ftpClient.enterLocalPassiveMode();
		ftpClient.retrieveFile("sn.last", fout);
		fout.close();
		ftpClient.disconnect();
		
        return fout.toByteArray();
    }

    protected void onResume() {
    	super.onResume();
    	
        locationManager = (LocationManager)getSystemService(Context.LOCATION_SERVICE);
        locationListener = new LocationListener() {
			@Override
			public void onLocationChanged(Location location) {
				Log.v(C.TAG, location.toString());
				LatLon pos = new LatLon(location.getLatitude(), location.getLongitude());
				mRadarView.setLocation(pos);
			}

			@Override
			public void onProviderDisabled(String provider) {}

			@Override
			public void onProviderEnabled(String provider) {}

			@Override
			public void onStatusChanged(String provider, int status, Bundle extras) {
				if (status != LocationProvider.AVAILABLE) {
					//TEMPORARILY_UNAVAILABLE
					//AVAILABLE
					//mRadarView.setLocation(null);
				}
			}
		};
		
        locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 0, 0, locationListener);
        //locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, locationListener);
    	
    	mRadarView.onResume();
    }
    
    protected void onPause() {
    	super.onPause();
    	
    	locationManager.removeUpdates(locationListener);
    	mRadarView.setLocation(null);
    	
    	mRadarView.onPause();
    }
    
	
	private class LoadShapefiles extends AsyncTask<Void, Void, Void> {
		private LatLon mCenter;
	
		public LoadShapefiles(LatLon center) {
			mCenter = center;

		}

		private ShapefileLayer loadShapefile(ShapefileConfig config) {

			try {
				saveResource(config.resShp, "temp.shp");
				saveResource(config.resDbf, "temp.dbf");
				saveResource(config.resShx, "temp.shx");
			} catch (IOException e) {
				Log.e(C.TAG, "Failed to save resource", e);
				return null;
			}
			
			Shapefile shapefile = new Shapefile();
			shapefile.open(getFilesDir() + "/" + "temp.shp");
			try {
				return new ShapefileLayer(mCenter, shapefile, config);
			} finally {
				shapefile.close();
				
				deleteResource("temp.shp");
				deleteResource("temp.dbf");
				deleteResource("temp.shx");
			}
			
		}
		
		@Override
		protected Void doInBackground(Void... params) {
			ShapefileLayer layer;
			
			ShapefileConfig[] configs = new ShapefileConfig[] {
					new ShapefileConfig(R.raw.states_shp, R.raw.states_dbf, R.raw.states_shx,
							3.0f, new Color(1.0f, 1.0f, 1.0f)),
					new ShapefileConfig(R.raw.counties_shp, R.raw.counties_dbf, R.raw.counties_shx,
							1.0f, new Color(0.75f, 0.75f, 0.75f))
			};
			
			for (ShapefileConfig config : configs) {
				layer = loadShapefile(config);
				if (layer == null)
					continue;
				
				mRadarView.addLayer(layer);
			}
			
			return null;
		}
		
		@Override
		protected void onPostExecute(Void res) {
			mLayersLoaded = true;
		}
		
		private void deleteResource(String name) {
			deleteFile(name);
		}

		private void saveResource(int resource, String name) throws IOException {
			InputStream fin = getResources().openRawResource(resource);
			OutputStream fout = openFileOutput(name, MODE_PRIVATE);
			byte[] buffer = new byte[8192];
			int read;
			while ((read = fin.read(buffer)) > 0) {
				fout.write(buffer, 0, read);
			}
			fin.close();
			fout.close();
		}

	}
    
}
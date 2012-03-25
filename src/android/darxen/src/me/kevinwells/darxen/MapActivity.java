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
    
    private ShapefileLayer mLayer;
	
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
        
        if (mLayer == null) {
        	new LoadShapefile(new LatLon(file.description.lat, file.description.lon),
        			R.raw.states_shp, R.raw.states_dbf, R.raw.states_shx,
        			"states.shp", "states.dbf", "states.shx").execute();
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
    
	
	private class LoadShapefile extends AsyncTask<Void, Void, ShapefileLayer> {
		private LatLon mCenter;
		
		private int idShp;
		private int idDbf;
		private int idShx;
		
		private String nameShp;
		private String nameDbf;
		private String nameShx;
		
		public LoadShapefile(LatLon center, int idShp, int idDbf, int idShx, String nameShp,
				String nameDbf, String nameShx) {
			mCenter = center;
			this.idShp = idShp;
			this.idDbf = idDbf;
			this.idShx = idShx;
			this.nameShp = nameShp;
			this.nameDbf = nameDbf;
			this.nameShx = nameShx;
		}

		@Override
		protected ShapefileLayer doInBackground(Void... params) {
			
			try {
				saveResource(idShp, nameShp);
				saveResource(idDbf, nameDbf);
				saveResource(idShx, nameShx);
			} catch (IOException e) {
				Log.e(C.TAG, "Failed to save resource", e);
				return null;
			}
			
			Shapefile shapefile = new Shapefile();
			shapefile.open(getFilesDir() + "/" + nameShp);
			try {
				return new ShapefileLayer(mCenter, shapefile);
			} finally {
				shapefile.close();
				
				deleteResource(nameShp);
				deleteResource(nameDbf);
				deleteResource(nameShx);
			}
		}
		
		@Override
		protected void onPostExecute(ShapefileLayer layer) {
			mLayer = layer;
			mRadarView.addLayer(layer);
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
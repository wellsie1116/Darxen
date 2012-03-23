package me.kevinwells.darxen;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.SocketException;

import me.kevinwells.darxen.data.DataFile;
import me.kevinwells.darxen.data.Level3Parser;
import me.kevinwells.darxen.data.ParseException;

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPReply;

import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.Bundle;
import android.util.Log;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.actionbarsherlock.app.SherlockActivity;

public class MapActivity extends SherlockActivity {
	
	private RadarView mRadarView;
	
    private LocationManager locationManager;
    private LocationListener locationListener;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mRadarView = new RadarView(this);
        ((FrameLayout)findViewById(R.id.container)).addView(mRadarView);
        
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
        title.setText(new String(file.header));
        
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
    
}
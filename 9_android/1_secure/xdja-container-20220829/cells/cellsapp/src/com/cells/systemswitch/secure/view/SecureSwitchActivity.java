package com.cells.systemswitch.secure.view;

import android.util.Log;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.os.SystemProperties;
import android.os.ServiceManager;
import java.io.IOException;
import android.os.RemoteException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import android.widget.Toast;

import android.content.pm.PackageManager;
import android.content.ComponentName;

import com.cells.systemswitch.secure.R;

import android.app.CellsPrivateServiceMgr;

import android.bluetooth.BluetoothAdapter;
import android.nfc.NfcAdapter;

import java.lang.RuntimeException;

public class SecureSwitchActivity extends Activity {
	private static final String TAG = "SecureSwitchActivity";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		if(!SystemProperties.get("ro.boot.vm","1").equals("0")){
			CellsPrivateServiceMgr mCellsService;
			mCellsService = new CellsPrivateServiceMgr(SecureSwitchActivity.this, 
					ServiceManager.getService("CellsPrivateService"));
			
			String name = (SystemProperties.get("ro.boot.vm.name","").equals("cell1")?"cell2":"cell1");
			try{
				disableAdapter();
				mCellsService.switchCellsVm(name);
			}catch(RuntimeException e){
				e.printStackTrace();
			}

			finish();
		}else{
			setContentView(R.layout.activity_main);

			ComponentName mDefault = getComponentName();
			ComponentName mIcon1 = new ComponentName(getBaseContext(),"com.cells.systemswitch.secure.TestIcon1");
			PackageManager mPm = getApplicationContext().getPackageManager();
			mPm.setComponentEnabledSetting(mDefault,
				PackageManager.COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP);
			mPm.setComponentEnabledSetting(mIcon1,
				PackageManager.COMPONENT_ENABLED_STATE_ENABLED, PackageManager.DONT_KILL_APP);
		}
	}

	public void btncell1(View v)
	{
		CellsPrivateServiceMgr mCellsService;
		mCellsService = new CellsPrivateServiceMgr(SecureSwitchActivity.this, 
																					ServiceManager.getService("CellsPrivateService"));

		if(SystemProperties.get("persist.sys.cell1.init").equals("0")){
			long beginTime=System.currentTimeMillis();
			try{
				//mCellsService.untarCellsVm("cell1");
				mCellsService.startCellsVm("cell1");
			}catch(RuntimeException e){
				e.printStackTrace();
			}
			long ms = System.currentTimeMillis() - beginTime;
			Log.e(TAG, "Boot image consumption - " + ms + "(ms).");

			int count = 20;
			while(SystemProperties.get("persist.sys.cell1.init").equals("0") && count > 0){
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				count--;
			}
		}

		try {
			disableAdapter();
			mCellsService.switchCellsVm("cell1");
		}catch(RuntimeException e){
			e.printStackTrace();
		}

		finish();
	}

	public void btncell2(View v)
	{
		CellsPrivateServiceMgr mCellsService;
		mCellsService = new CellsPrivateServiceMgr(SecureSwitchActivity.this, 
																					ServiceManager.getService("CellsPrivateService"));

		if(SystemProperties.get("persist.sys.cell2.init").equals("0")){
			long beginTime=System.currentTimeMillis();
			try{
				//mCellsService.untarCellsVm("cell2");
				mCellsService.startCellsVm("cell2");
			}catch(RuntimeException e){
				e.printStackTrace();
			}
			long ms = System.currentTimeMillis() - beginTime;
			Log.e(TAG, "Boot image consumption - " + ms + "(ms).");

			int count = 20;
			while(SystemProperties.get("persist.sys.cell2.init").equals("0") && count > 0){
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				count--;
			}
		}

		try {
			disableAdapter();
			mCellsService.switchCellsVm("cell2");
		}catch(RuntimeException e){
			e.printStackTrace();
		}

		finish();
	}

	public void btncell3(View v)
	{
		CellsPrivateServiceMgr mCellsService;
		mCellsService = new CellsPrivateServiceMgr(SecureSwitchActivity.this, 
																					ServiceManager.getService("CellsPrivateService"));

		if(SystemProperties.get("persist.sys.cell3.init").equals("0")){
			long beginTime=System.currentTimeMillis();
			try{
				//mCellsService.untarCellsVm("cell3");
				mCellsService.startCellsVm("cell3");
			}catch(RuntimeException e){
				e.printStackTrace();
			}
			long ms = System.currentTimeMillis() - beginTime;
			Log.e(TAG, "Boot image consumption - " + ms + "(ms).");

			int count = 20;
			while(SystemProperties.get("persist.sys.cell3.init").equals("0") && count > 0){
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				count--;
			}
		}

		try {
			disableAdapter();
			mCellsService.switchCellsVm("cell3");
		}catch(RuntimeException e){
			e.printStackTrace();
		}

		finish();
	}

	public void btncell4(View v)
	{
		CellsPrivateServiceMgr mCellsService;
		mCellsService = new CellsPrivateServiceMgr(SecureSwitchActivity.this, 
																					ServiceManager.getService("CellsPrivateService"));

		if(SystemProperties.get("persist.sys.cell4.init").equals("0")){
			long beginTime=System.currentTimeMillis();
			try{
				//mCellsService.untarCellsVm("cell4");
				mCellsService.startCellsVm("cell4");
			}catch(RuntimeException e){
				e.printStackTrace();
			}
			long ms = System.currentTimeMillis() - beginTime;
			Log.e(TAG, "Boot image consumption - " + ms + "(ms).");

			int count = 20;
			while(SystemProperties.get("persist.sys.cell4.init").equals("0") && count > 0){
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				count--;
			}
		}

		try {
			disableAdapter();
			mCellsService.switchCellsVm("cell4");
		}catch(RuntimeException e){
			e.printStackTrace();
		}

		finish();
	}

	public void btncell5(View v)
	{
		CellsPrivateServiceMgr mCellsService;
		mCellsService = new CellsPrivateServiceMgr(SecureSwitchActivity.this, 
																					ServiceManager.getService("CellsPrivateService"));

		if(SystemProperties.get("persist.sys.cell5.init").equals("0")){
			long beginTime=System.currentTimeMillis();
			try{
				//mCellsService.untarCellsVm("cell5");
				mCellsService.startCellsVm("cell5");
			}catch(RuntimeException e){
				e.printStackTrace();
			}
			long ms = System.currentTimeMillis() - beginTime;
			Log.e(TAG, "Boot image consumption - " + ms + "(ms).");

			int count = 20;
			while(SystemProperties.get("persist.sys.cell5.init").equals("0") && count > 0){
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				count--;
			}
		}

		try {
			disableAdapter();
			mCellsService.switchCellsVm("cell5");
		}catch(RuntimeException e){
			e.printStackTrace();
		}

		finish();
	}

	private void disableAdapter(){
		BluetoothAdapter blueadapter = BluetoothAdapter.getDefaultAdapter();
		if(blueadapter != null)
			blueadapter.disable();
		NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(SecureSwitchActivity.this);
		if(nfcAdapter != null)
			nfcAdapter.disable();
	}

}

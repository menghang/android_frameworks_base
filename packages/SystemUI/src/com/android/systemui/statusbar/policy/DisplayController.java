/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.systemui.statusbar.policy;

import java.util.ArrayList;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.util.Slog;
import android.widget.ImageView;
import android.widget.TextView;
import android.view.DisplayManager;
import com.android.systemui.statusbar.policy.DisplayHotPlugPolicy;
import android.media.MediaPlayer;
import android.media.AudioSystem;
import com.android.systemui.R;
import android.os.SystemProperties;
import android.provider.Settings;

public class DisplayController extends BroadcastReceiver {
    private static final String TAG = "StatusBar.DisplayController";

    private Context mContext;
    private final  DisplayManager mDisplayManager;
	private DisplayHotPlugPolicy  mDispHotPolicy = null;
	private static final boolean SHOW_HDMIPLUG_IN_CALL = true;
    private static final boolean SHOW_TVPLUG_IN_CALL = true;

    public DisplayController(Context context) {
        mContext = context;

		mDisplayManager = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
		mDispHotPolicy = new StatusBarPadHotPlug();
		
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_HDMISTATUS_CHANGED);
		filter.addAction(Intent.ACTION_TVDACSTATUS_CHANGED);
        context.registerReceiver(this, filter);
    }

    public void onReceive(Context context, Intent intent) 
    {
        final String action = intent.getAction();
        if (action.equals(Intent.ACTION_HDMISTATUS_CHANGED)) 
        {
            mDispHotPolicy.onHdmiPlugChanged(intent);
        }
        else if(action.equals(Intent.ACTION_TVDACSTATUS_CHANGED))
        {
			mDispHotPolicy.onTvDacPlugChanged(intent);
        }
    }
        
    private class StatusBarPadHotPlug implements DisplayHotPlugPolicy
    {
        //0:screen0 fix;
        //3:screen0 fix,screen1 auto,(same ui,one video on screen1);
        //4:screen0 fix,screen1 auto,(same ui,two video);
        //5:screen0 auto(ui use fe)
        //6:screen0 auto(ui use be)
        //7:screen0 auto(fb var)
        //8:screen0 auto(ui use be ,use gpu scaler)
        public int mDisplay_mode = 3;

    	StatusBarPadHotPlug()
    	{
    	}
    	
    	private void onHdmiPlugIn(Intent intent) 
		{
			int     maxscreen;
			int     maxhdmimode;
			int     customhdmimode;
			int     hdmi_mode;
			int     AUTO_HDMI_MODE = 0xff;
			int     MIN_HDMI_MODE = 0;
			
	        if (SHOW_HDMIPLUG_IN_CALL) 
			{
	          	Slog.d(TAG,"onHdmiPlugIn Starting!\n");

	            if(mDisplay_mode == 4)
	            {
	                hdmi_mode = DisplayManager.DISPLAY_TVFORMAT_720P_50HZ;
	            }
	            else
	            {
	                customhdmimode = Settings.System.getInt(mContext.getContentResolver(), Settings.System.HDMI_OUTPUT_MODE, AUTO_HDMI_MODE);
				    maxhdmimode	= mDisplayManager.getMaxHdmiMode();
                    if (customhdmimode < AUTO_HDMI_MODE) {
                        hdmi_mode = customhdmimode;
                    } else {
                        hdmi_mode = maxhdmimode;
                    }
				}

				if(mDisplay_mode == 3)
				{
					mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_HDMI,hdmi_mode);
					mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_DUALSAME);
                    SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_AUX_DIGITAL));
                    AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_AUX_DIGITAL);
				}
				else if(mDisplay_mode == 4)
				{
					mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_HDMI,hdmi_mode);
					mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_DUALSAME_TWO_VIDEO);
                    SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_AUX_DIGITAL));
                    AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_AUX_DIGITAL);
				}
				else if(mDisplay_mode == 5)
				{
					mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_HDMI,hdmi_mode);
					mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR);
                    SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_AUX_DIGITAL));
                    AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_AUX_DIGITAL);
				}
				else if(mDisplay_mode == 6)
				{
					mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_HDMI,hdmi_mode);
					mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR_BE);
                    SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_AUX_DIGITAL));
                    AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_AUX_DIGITAL);
				}
				else if(mDisplay_mode == 7)
				{
					mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_HDMI,hdmi_mode);
					mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_FB_VAR);
                    SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_AUX_DIGITAL));
                    AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_AUX_DIGITAL);
				}
				else if(mDisplay_mode == 8)
				{
					mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_HDMI,hdmi_mode);
					mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_FB_GPU);
                    SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_AUX_DIGITAL));
                    AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_AUX_DIGITAL);
				}
	        }
	    }
	
		private void onTvDacYPbPrPlugIn(Intent intent)
		{
            if(mDisplay_mode == 3)
            {
                mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_720P_50HZ);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_DUALSAME);
            }
            else if(mDisplay_mode == 4)
            {
                mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_720P_50HZ);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_DUALSAME_TWO_VIDEO);
            }
            else if(mDisplay_mode == 5)
            {
                mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_720P_50HZ);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR);
            }
            else if(mDisplay_mode == 6)
            {
                mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_720P_50HZ);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR_BE);
            }
		}
		
		private void onTvDacCVBSPlugIn(Intent intent)
		{
            if(mDisplay_mode == 3)
            {
                mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_NTSC);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_DUALSAME);
            }
            else if(mDisplay_mode == 4)
            {
                mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_NTSC);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_DUALSAME_TWO_VIDEO);
            }
            else if(mDisplay_mode == 5)
            {
                mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_NTSC);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR);
            }
            else if(mDisplay_mode == 6)
            {
                mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_TV,DisplayManager.DISPLAY_TVFORMAT_NTSC);
                mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR_BE);
            }
		}
	
		private void onHdmiPlugOut(Intent intent)
		{
			int     maxscreen;
			
			Slog.d(TAG,"onHdmiPlugOut Starting!\n");

			if((mDisplay_mode == 3) || (mDisplay_mode == 4))
			{
			    mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_NONE,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE);
                SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_SPEAKER));
                AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_SPEAKER);
			}
			else if(mDisplay_mode == 5)
			{
    	      	mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_LCD,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR);
                SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_SPEAKER));
                AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_SPEAKER);
	        }
	        else if(mDisplay_mode == 6)
			{
    	      	mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_LCD,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR_BE);
                SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_SPEAKER));
                AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_SPEAKER);
	        }
	        else if(mDisplay_mode == 7)
			{
    	      	mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_LCD,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_FB_VAR);
                SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_SPEAKER));
                AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_SPEAKER);
	        }
	        else if(mDisplay_mode == 8)
			{
    	      	mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_LCD,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_FB_GPU);
                SystemProperties.set("audio.routing", Integer.toString(AudioSystem.DEVICE_OUT_SPEAKER));
                AudioSystem.setParameters("routing="+AudioSystem.DEVICE_OUT_SPEAKER);
	        }
		}
	
		private void onTvDacPlugOut(Intent intent)
		{
			Slog.d(TAG,"onTvDacPlugOut Starting!\n");
			
			if((mDisplay_mode == 3) || (mDisplay_mode == 4))
			{
			    mDisplayManager.setDisplayParameter(1,DisplayManager.DISPLAY_OUTPUT_TYPE_NONE,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE);
			}
			else if(mDisplay_mode == 5)
			{
    	      	mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_LCD,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR);
	        }
	        else if(mDisplay_mode == 6)
			{
    	      	mDisplayManager.setDisplayParameter(0,DisplayManager.DISPLAY_OUTPUT_TYPE_LCD,0);
    	        mDisplayManager.setDisplayMode(DisplayManager.DISPLAY_MODE_SINGLE_VAR_BE);
	        }
		}
		
		public void onHdmiPlugChanged(Intent intent)
		{
			int   hdmiplug;
			
			hdmiplug = intent.getIntExtra(DisplayManager.EXTRA_HDMISTATUS, 0);
			if(hdmiplug == 1)
			{
				onHdmiPlugIn(intent);
			}
			else
			{
				onHdmiPlugOut(intent);
			}
		}
		
		public void onTvDacPlugChanged(Intent intent)
		{
			int   tvdacplug;
			
			tvdacplug = intent.getIntExtra(DisplayManager.EXTRA_TVSTATUS, 0);
			if(tvdacplug == 1)
			{
				onTvDacYPbPrPlugIn(intent);
			}
			else if(tvdacplug == 2)
			{
				onTvDacCVBSPlugIn(intent);
			}
			else
			{
				onTvDacPlugOut(intent);
			}
		}
    }
    
    private class StatusBarTVDHotPlug implements DisplayHotPlugPolicy
    {
    	StatusBarTVDHotPlug()
    	{
    		
    	}

		public void onHdmiPlugChanged(Intent intent)
		{
		}
		
		public void onTvDacPlugChanged(Intent intent)
		{
		}
    }
}

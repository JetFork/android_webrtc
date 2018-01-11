/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtc.videoengineapp;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.regex.Pattern;

import org.webrtc.videoengine.ViERenderer;
import org.webrtc.videoengine.VideoCaptureAndroid;

import android.app.AlertDialog;
import android.app.TabActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;
import android.widget.TextView;

/** {@} */
@SuppressWarnings("deprecation")
public class WebRTCDemo extends TabActivity implements IViEAndroidCallback,
                                                View.OnClickListener,
                                                OnItemSelectedListener {
    private ViEAndroidJavaAPI vieAndroidAPI = null;
    private VideoCaptureAndroid vieCapture = null;

    // remote renderer
    private SurfaceView remoteSurfaceView = null;
    private SurfaceView remoteSurfaceView2 = null;
    // local renderer and camera
    private SurfaceView svLocal = null;
    // channel number
    private int channel;
    private int channel2;
    private int cameraId;
    private int voiceChannel = -1;
    private int callBack = 8888;

    // flags
    private boolean viERunning = false;
    private boolean voERunning = false;

    // debug
    private boolean enableTrace = true;
    
    // cpu 核数
    private int numCores = 1;
    private long cpuFrequence = 0;
    private int touchCount = 0;
    private int firstClick = 0;
    private int secondClick = 0;
    private Button fullScreen;
    private int layHeight = 0;
    private int layWidth = 0;
    private LayoutParams layout;
    WindowManager wManager;
    DisplayMetrics metrics;

    // Constant
    private static final String TAG = "yyf";
    private static final int RECEIVE_CODEC_FRAMERATE = 10;
    private static final int SEND_CODEC_FRAMERATE = 12;
    private static final int INIT_BITRATE = 1500;
    private static final String LOOPBACK_IP = "127.0.0.1";

    private TabHost mTabHost = null;

    private LinearLayout mLlRemoteSurface = null;
    private LinearLayout mLlRemoteSurface2 = null;
    private LinearLayout mLlLocalSurface = null;

    private Button btSwitchCamera;
    private Button btStartCapture;
    private Button btStartSend;
    private Button btStartReceive;
    private Button btStartPlayFlv;
    private Button btSnapLocal;
    private Button btSnapRemote;
    private Button btSetCaptureFlashlight;
    
    // audio
    private Button btAudioSpeex;
    private Button btAudioAmr;
    private Button btAudioOpus;
    private Button btAudioIlbc;
    private Button btAudioStop;

    private EditText etRemoteIp;
    private EditText etRemotePort;
    private String remoteIp = "";
    public enum RenderType {
        OPENGL,
        SURFACE,
        MEDIACODEC
    }
    RenderType renderType = RenderType.OPENGL;
    //RenderType renderType = RenderType.SURFACE;

    // Video settings
    private Spinner spCodecType;
    private int codecType = 0;
    private Spinner spCodecSize;
    private int codecSizeWidth = 0;
    private int codecSizeHeight = 0;

    // Audio settings
    private Spinner spVoiceCodecType;
    private int voiceCodecType = 0;
    private CheckBox cbVoice;
    private boolean enableVoice = false;
    private CheckBox cbEnableSpeaker;
    private boolean enableSpeaker = false;
    private CheckBox cbEnableAGC;
    private boolean enableAGC = false;
    private CheckBox cbEnableAECM;
    private boolean enableAECM = false;
    private CheckBox cbEnableNS;
    private boolean enableNS = false;
    private int voiceSendPort = 0;

    // Stats variables
    private int frameRateI;
    private int bitRateI;
    private int packetLoss;
    private int frameRateO;
    private int bitRateO;
    private int numCalls = 0;

    // Variable for storing variables
    private String webrtcName = "/webrtc";
    private String webrtcDebugDir = null;

    private boolean usingFrontCamera = true;
    private boolean usingFlashlight = false;

    private String[] mVideoCodecsStrings = null;
    private String[] mVideoCodecsSizeStrings = { "176x144", "320x240",
                                                 "352x288", "640x480" };
    
    private String[] mVoiceCodecsStrings = null;


    private OrientationEventListener orientationListener;
    int currentOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    int currentCameraOrientation = 0;
    
    //private BaseGuideView testView;



    private BroadcastReceiver receiver;

    private TextView tview = null;
    
    private OrientationEventListener mOrientationListener;
 // screen orientation listener
 private int mScreenProtrait = 0;
 private int mCurrentOrient = 0;
 private int curOrientation;
 protected void OrientationChanged(int orientation)                                                                                             
 {
	 Log.e("kxw", "onConfigurationChanged:"+orientation);
	 curOrientation = (orientation+(usingFrontCamera?0:1)*180)%360;
	 Log.e("kxw", "onConfigurationChanged:"+curOrientation);
	 //int newRotation = getCameraOrientation(currentCameraOrientation);
	 //Log.e("kxw", "onConfigurationChanged:"+ newRotation);
     if (viERunning) {
    	 //testView.setOrientation(currentCameraOrientation, true);
         vieAndroidAPI.SetRotation(cameraId, curOrientation);
         //vieCapture.SetPreviewRotation(curOrientation);
         //mLlRemoteSurface.removeView()
         //mLlRemoteSurface.setRotation(curOrientation);
         //remoteSurfaceView.setRotation(curOrientation);
         //mLlLocalSurface.removeView(svLocal);
         //svLocal = null;
         //mLlLocalSurface.setRotation(curOrientation);
         //svLocal = ViERenderer.CreateLocalRenderer(this);
         //svLocal.setRotation(curOrientation);
         //mLlLocalSurface.addView(svLocal);
         
     }
 }
 //screen orientation change event
 //最后，自定义监听类
 private final void startOrientationChangeListener() {
	  mOrientationListener = new OrientationEventListener(this) {
	  @Override
	  public void onOrientationChanged(int rotation) {
	  if (((rotation >= 0) && (rotation <= 35)) || (rotation >= 320))
	  {//portrait
		  mCurrentOrient = 270;
		  if(mCurrentOrient!=mScreenProtrait)
		  {
			  mScreenProtrait = mCurrentOrient;
			  //OrientationChanged(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
			  OrientationChanged(270);
			  //Log.d(TAG, "Screen orientation changed from Landscape to Portrait!");
		  }
	  }
	  else if(((rotation>=135)&&(rotation<=220)))
	  {
		  mCurrentOrient = 90;
		  if(mCurrentOrient!=mScreenProtrait)
		  {
			  mScreenProtrait = mCurrentOrient;
			  OrientationChanged(90);
			  //Log.d(TAG, "Screen orientation changed from Portrait to Landscape!");
		  }
	  }
	  else if (((rotation > 50) && (rotation < 120)))
	  {//landscape
		  mCurrentOrient = 180;
		  if(mCurrentOrient!=mScreenProtrait)
		  {
			  mScreenProtrait = mCurrentOrient;
			  OrientationChanged(180);
			  //Log.d(TAG, "Screen orientation changed from Portrait to Landscape!");
		  }
	  }
	  else if(((rotation>230)&&(rotation<310)))
	  {
		  mCurrentOrient = 0;
		  if(mCurrentOrient!=mScreenProtrait)
		  {
			  mScreenProtrait = mCurrentOrient;
			  OrientationChanged(0);
			  //Log.d(TAG, "Screen orientation changed from Portrait to Landscape!");
		  }
	  }
	  }
	  };
	  mOrientationListener.enable();
 }
    
    
    private Handler handler = new Handler();
    private Runnable statViewCallback = new Runnable() {
            public void run() {
            	statView();
            }
        };
    //cpu核数
    private int GetNumCores() {
    	 class CpuFilter implements FileFilter{
    	       public boolean accept(File pathname){
    		   if(Pattern.matches("cpu[0-9]", pathname.getName())){
    		       return true;
    		   }
    			   return false;
    		 }
    	   }
    	   try {
    		   File dir = new File("/sys/devices/system/cpu");
    		   File[] files = dir.listFiles(new CpuFilter());
    		   Log.d(TAG, "CPU Count: "+files.length);
    		   return files.length;
    	   }catch (Exception e) {
    		   Log.d(TAG, "CPU Count: Failed.");
    		   e.printStackTrace();
    		   return 1;
    	   }
        }
    //cpu频率
    public static long GetCpuFrequence() {
    	ProcessBuilder cmd;
    	try{
    		String[] args = { "/system/bin/cat",
    				"/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
    		};
    		cmd = new ProcessBuilder(args);
            Process process = cmd.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader
        (
        		process.getInputStream()));
            String line = reader.readLine();
            //return StringUtils.parseLongSafe(line, 10, 0);
            return Long.parseLong(line, 10);
    	} catch (IOException ex) {
    		ex.printStackTrace();
    	}
    	return 0;
    }
private void statView()
{
	tview.setText("running");
	String loadText;
    loadText = "in:" + frameRateI + " fps\n" + bitRateI + "k bps/ " + packetLoss +"\n";
    //canvas.drawText(loadText, 4, 172, loadPaint);
    loadText += "out:" + frameRateO + " fps\n " + bitRateO + "k bps"+"\n";
    //canvas.drawText(loadText, 4, 192, loadPaint);
    tview.setText(loadText);
    handler.postDelayed(statViewCallback,1000);
    
}
 //   private SensorManager mManager = null;
//    private Sensor mSensor = null;
    //2、创建监听器
    //在 Activity 中定义以下成员变量：
 //   private SensorEventListener mListener = null;
    
    public int getCameraOrientation(int cameraOrientation) {
        Display display = this.getWindowManager().getDefaultDisplay();
        int displatyRotation = display.getRotation();
        int degrees = 0;
        switch (displatyRotation) {
            case Surface.ROTATION_0: degrees = 0; break;
            case Surface.ROTATION_90: degrees = 90; break;
            case Surface.ROTATION_180: degrees = 180; break;
            case Surface.ROTATION_270: degrees = 270; break;
        }
        int result = 0;
        if (cameraOrientation > 180) {
            result = (cameraOrientation + degrees) % 360;
        } else {
            result = (cameraOrientation - degrees + 360) % 360;
        }
        return result;
    }

    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
   	    Log.e("kxw", "onConfigurationChanged:");
        int newRotation = getCameraOrientation(currentCameraOrientation);
        if (viERunning) {
            vieAndroidAPI.SetRotation(cameraId, newRotation);
        }
    }

    // Called when the activity is first created.

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.e("kxw", "onCreate");
        super.onCreate(savedInstanceState);
        numCores = GetNumCores();
        wManager = getWindowManager();
        metrics = new DisplayMetrics();
        wManager.getDefaultDisplay().getMetrics(metrics);

        cpuFrequence = GetCpuFrequence();
        Log.e("kxw", getIntent().getStringExtra("org.webrtc.videoengineapp.serverIP")); 
       Log.e("kxw", getIntent().getStringExtra("org.webrtc.videoengineapp.groupID")); 
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
               WindowManager.LayoutParams.FLAG_FULLSCREEN);
        // Set screen orientation
        //setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        //setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
       setContentView(R.layout.single_tableview);
       fullScreen = (Button) findViewById(R.id.btFullScreen);
       fullScreen.setOnClickListener(this);
        //setContentView(R.layout.login_layout);
       
       IntentFilter receiverFilter = new IntentFilter(Intent.ACTION_HEADSET_PLUG);

       receiver = new BroadcastReceiver() {
               @Override
               public void onReceive(Context context, Intent intent) {
                   if (intent.getAction().compareTo(Intent.ACTION_HEADSET_PLUG)
                           == 0) {
                       int state = intent.getIntExtra("state", 0);
                       Log.v(TAG, "Intent.ACTION_HEADSET_PLUG state: " + state +
                               " microphone: " + intent.getIntExtra("microphone", 0));
                       if (voERunning) {
                           if (state == 1) {
                               enableSpeaker = true;
                           } else {
                               enableSpeaker = false;
                           }
                           routeAudio(enableSpeaker);
                       }
                   }
               }
           };
       registerReceiver(receiver, receiverFilter);
        
     mTabHost = getTabHost();
        // Main tab
        TabSpec mTabSpecVideo = mTabHost.newTabSpec("tab_1");
        mTabSpecVideo.setIndicator("Main");
        mTabSpecVideo.setContent(R.id.tab_video);
        mTabHost.addTab(mTabSpecVideo);

        TabSpec mTabSpecConfig = mTabHost.newTabSpec("tab_2");
        mTabSpecConfig.setIndicator("Settings");
        mTabSpecConfig.setContent(R.id.tab_config);
        mTabHost.addTab(mTabSpecConfig);

        TabSpec mTabv;
        mTabv = mTabHost.newTabSpec("tab_3");
        mTabv.setIndicator("Video");
        mTabv.setContent(R.id.tab_vconfig);
        mTabHost.addTab(mTabv);
        
        TabSpec mTaba;
        mTaba = mTabHost.newTabSpec("tab_4");
        mTaba.setIndicator("Audio");
        mTaba.setContent(R.id.tab_aconfig);
        mTabHost.addTab(mTaba); 
        
       

        int childCount = mTabHost.getTabWidget().getChildCount();
        for (int i = 0; i < childCount; i++) {
            mTabHost.getTabWidget().getChildAt(i).getLayoutParams().height = 50;
        }


        // Create a folder named webrtc in /scard for debugging
        webrtcDebugDir = Environment.getExternalStorageDirectory().toString() +
                webrtcName;
        File webrtcDir = new File(webrtcDebugDir);
        if (!webrtcDir.exists() && webrtcDir.mkdir() == false) {
            Log.v(TAG, "Failed to create " + webrtcDebugDir);
        } else if (!webrtcDir.isDirectory()) {
            Log.v(TAG, webrtcDebugDir + " exists but not a folder");
            webrtcDebugDir = null;
        }
        
        startOrientationChangeListener();
     /* 测试文件读写 
      * File aFile = new File(webrtcDebugDir+"myFile.txt");
		FileOutputStream outputFile = null;
		System.out.println(aFile);
		try
		{
			outputFile = new FileOutputStream(aFile);
			PrintStream p = new PrintStream(outputFile);
			for (int i = 0; i < 5; i++)
			{
				p.printf("Hi,  I am testing webrtc :%d x 3 = %d.\n", i, i * 3);
			}
			outputFile.close();

		} catch (FileNotFoundException e)
		{
			e.printStackTrace(System.err);
			System.out.println(e.getMessage());
		}catch (IOException e)
		{
			e.printStackTrace(System.err);
			System.out.println(e.getMessage());
		}
		*/
        
        
        
        startMain();

     //   if (AUTO_CALL_RESTART_DELAY_MS > 0)
       //     startOrStop();
        
        
    /* 测试手机屏幕常亮 
     * PowerManager pm = (PowerManager) this.getSystemService(
          Context.POWER_SERVICE);
      wakeLock = pm.newWakeLock(
          PowerManager.SCREEN_DIM_WAKE_LOCK, TAG);*/

        

     /* 测试消息广播  
      * IntentFilter receiverFilter = new IntentFilter(Intent.ACTION_HEADSET_PLUG);

        receiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    if (intent.getAction().compareTo(Intent.ACTION_HEADSET_PLUG)
                            == 0) {
                        int state = intent.getIntExtra("state", 0);
                        System.out.println("get headset:"+state);
                        if (voERunning) {
                            if (state == 1) {
                                enableSpeaker = true;
                            } else {
                                enableSpeaker = false;
                            }
                            routeAudio(enableSpeaker);
                        }
                    }
                }
            };
        registerReceiver(receiver, receiverFilter);
        String act = "kxw.actTest";
        IntentFilter filter = new IntentFilter();
        filter.addAction(act);

        receiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                  //  if (intent.getAction().compareTo("kxw.actTest")
                      //      == 0) {
                        int state = intent.getIntExtra("mymsg", 0);
                        System.out.println("get mymsg:"+state);
                    //}
                }
            };
        registerReceiver(receiver, filter);
        
        Intent intent = new Intent(act);
        intent.putExtra("mymsg", 6);
        sendBroadcast(intent );*/
        
        
        /*测试方向传感器  
          orientationListener =
        new OrientationEventListener(this, SensorManager.SENSOR_DELAY_UI) {
            public void onOrientationChanged (int orientation) {
                if (orientation != ORIENTATION_UNKNOWN) {
                    currentOrientation = orientation;
                    System.out.println("orientation changed:" +orientation );
                }
            }
        };
orientationListener.enable ();*/
        
        
      /*测试重力传感器 
       SensorManager mManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
      Sensor mSensor = mManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        //以下代码加入到 onCreate() 方法中：
      SensorEventListener mListener = new SensorEventListener() {
        	@Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
        	@Override
        public void onSensorChanged(SensorEvent event) {
        float x = event.values[SensorManager.DATA_X];
        float y = event.values[SensorManager.DATA_Y];
        float z = event.values[SensorManager.DATA_Z];
        System.out.println("x,y,z:"+x+","+y+","+z);
        }
        };
        mManager.registerListener(mListener, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
        */
        
        
        /*view 重绘事件重写
        private class StatsView extends View{
            public StatsView(Context context){
                super(context);
            }

            @Override protected void onDraw(Canvas canvas) {
                super.onDraw(canvas);

                Paint loadPaint = new Paint();
                loadPaint.setAntiAlias(true);
                loadPaint.setTextSize(16);
                loadPaint.setARGB(255, 255, 255, 255);

                canvas.drawText("#calls " + numCalls, 4, 152, loadPaint);

                String loadText;
                loadText = "in:" + frameRateI + " fps/" + bitRateI + "k bps/ " + packetLoss;
                //canvas.drawText(loadText, 4, 172, loadPaint);
                loadText += "\nout:" + frameRateO + " fps/ " + bitRateO + "k bps";
                //canvas.drawText(loadText, 4, 192, loadPaint);
                tview.setText(loadText);
                updateDisplay();
            }

            void updateDisplay() {
                invalidate();
            }
        }
    */

    }
    @Override
    public void onPause(){
    	super.onPause();
    	System.out.println("myapp Paused...");
    }
    @Override
    public void onResume(){
    	super.onResume();
    	//setContentView(R.layout.single_main);
    	System.out.println("myapp Resume...");
    }
    // Called before the activity is destroyed.
    @Override
    public void onDestroy() {
        Log.e("kxw", "onDestroy");
        handler = null;//.removeCallbacks(statViewCallback);
        unregisterReceiver(receiver);
        stopAll();
        super.onDestroy();
    }

    private String getLocalIpAddress() {
        String localIPs = "";
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface
                         .getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr =
                             intf.getInetAddresses();
                     enumIpAddr.hasMoreElements(); ) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        localIPs +=
                                inetAddress.getHostAddress().toString() + " ";
                        // Set the remote ip address the same as
                        // the local ip address of the last netif
                        remoteIp = inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e(TAG, ex.toString());
        }
        return localIPs;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (viERunning) {
                stopAll();
                startMain();
            }
            finish();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void stopAll() {
        Log.d(TAG, "stopAll");

        if (vieAndroidAPI != null) {
        	
        	if (voERunning) {
                voERunning = false;
                stopVoiceEngine();
            }
        	
          //  if (viERunning) {
                viERunning = false;
                vieAndroidAPI.StopRender(channel);
                vieAndroidAPI.StopReceive(channel);
                vieAndroidAPI.StopRender(channel2);
                vieAndroidAPI.StopReceive(channel2);
                vieAndroidAPI.StopSend(channel);
                vieAndroidAPI.RemoveRemoteRenderer(channel);
                vieAndroidAPI.RemoveRemoteRenderer(channel2);
                vieAndroidAPI.StopCamera(cameraId);
                vieAndroidAPI.Terminate();
                mLlRemoteSurface.removeView(remoteSurfaceView);
                mLlRemoteSurface2.removeView(remoteSurfaceView2);
                mLlLocalSurface.removeView(svLocal);
                remoteSurfaceView = null;
                remoteSurfaceView2 = null;
                svLocal = null;
                vieAndroidAPI = null;
            //}
           
        }
    }

    /** {@ArrayAdapter} */
    public class SpinnerAdapter extends ArrayAdapter<String> {
        private String[] mCodecString = null;
        public SpinnerAdapter(Context context, int textViewResourceId, String[] objects) {
            super(context, textViewResourceId, objects);
            mCodecString = objects;
        }

        @Override public View getDropDownView(int position, View convertView, ViewGroup parent) {
            return getCustomView(position, convertView, parent);
        }

        @Override public View getView(int position, View convertView, ViewGroup parent) {
            return getCustomView(position, convertView, parent);
        }

        public View getCustomView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = getLayoutInflater();
            View row = inflater.inflate(R.layout.row, parent, false);
            TextView label = (TextView) row.findViewById(R.id.spinner_row);
            label.setText(mCodecString[position]);
            return row;
            
           
        }
    }

    private void startMain() {
    	Log.e("kxw", "startMain");
        mTabHost.setCurrentTab(0);

        mLlRemoteSurface = (LinearLayout) findViewById(R.id.llRemoteView);
        mLlRemoteSurface2 = (LinearLayout) findViewById(R.id.llRemoteView2);
        layHeight = mLlRemoteSurface.getLayoutParams().height;
        layWidth = mLlRemoteSurface.getLayoutParams().width;
        layout = (LayoutParams)mLlRemoteSurface.getLayoutParams();
        mLlLocalSurface = (LinearLayout) findViewById(R.id.llLocalView);
        //testView = new TestView(getApplicationContext(), null);
        //testView = new TestView(this, null);
        //mLlLocalSurface.setRotation(90);

        if (null == vieAndroidAPI) {
            vieAndroidAPI = new ViEAndroidJavaAPI(this);
        }
        Log.e("kxw", "GetVideoEngine");
        if (0 > setupVoE() || 0 > vieAndroidAPI.GetVideoEngine() ||
                0 > vieAndroidAPI.Init(enableTrace)) {
            // Show dialog
            AlertDialog alertDialog = new AlertDialog.Builder(this).create();
            alertDialog.setTitle("WebRTC Error");
            alertDialog.setMessage("Can not init video engine.");
            alertDialog.setButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        return;
                    } });
            alertDialog.show();
        }

        btSwitchCamera = (Button) findViewById(R.id.btSwitchCamera);
        btSwitchCamera.setOnClickListener(this);
        btStartCapture = (Button) findViewById(R.id.btStartCapture);
        btStartCapture.setOnClickListener(this);
        btStartSend = (Button) findViewById(R.id.btStartSend);
        btStartSend.setOnClickListener(this);
        btStartReceive = (Button) findViewById(R.id.btStartReceive);
        btStartReceive.setOnClickListener(this);
        btStartPlayFlv = (Button) findViewById(R.id.btStartPlayFlv);
        btStartPlayFlv.setOnClickListener(this);
        btSnapLocal = (Button) findViewById(R.id.btSnapLocal);
        btSnapLocal.setOnClickListener(this);
        btSnapRemote = (Button) findViewById(R.id.btSnapRemote);
        btSnapRemote.setOnClickListener(this);
        btSetCaptureFlashlight = (Button) findViewById(R.id.btSetCaptureFlashlight);
        btSetCaptureFlashlight.setOnClickListener(this);
        
        // audio
        btAudioSpeex = (Button) findViewById(R.id.btAudioSpeex);
        btAudioSpeex.setOnClickListener(this);
        btAudioAmr = (Button) findViewById(R.id.btAudioAmr);
        btAudioAmr.setOnClickListener(this);
        btAudioOpus = (Button) findViewById(R.id.btAudioOpus);
        btAudioOpus.setOnClickListener(this);
        btAudioIlbc = (Button) findViewById(R.id.btAudioIlbc);
        btAudioIlbc.setOnClickListener(this);
        btAudioStop = (Button) findViewById(R.id.btAudioStop);
        btAudioStop.setOnClickListener(this);
        
        findViewById(R.id.btExit).setOnClickListener(this);
        tview = (TextView)findViewById(R.id.statView);
       tview.setText("hi, tview");
        // cleaning
        remoteSurfaceView = null;
        remoteSurfaceView2 = null;
        svLocal = null;
        
        // Video codec
        Log.e("*WEBRTCJ*", "GetCodecs");
       
        mVideoCodecsStrings = vieAndroidAPI.GetCodecs();
        spCodecType = (Spinner) findViewById(R.id.spCodecType);
        spCodecType.setOnItemSelectedListener(this);
        spCodecType.setAdapter(new SpinnerAdapter(this,
                        R.layout.row,
                        mVideoCodecsStrings));
        spCodecType.setSelection(1);

        // Video Codec size
        spCodecSize = (Spinner) findViewById(R.id.spCodecSize);
        spCodecSize.setOnItemSelectedListener(this);
        spCodecSize.setAdapter(new SpinnerAdapter(this,
                        R.layout.row,
                        mVideoCodecsSizeStrings));
        spCodecSize.setSelection(mVideoCodecsSizeStrings.length - 1);

        // Voice codec
        Log.e("*WEBRTCJ*", "VoE_GetCodecs");
        mVoiceCodecsStrings = vieAndroidAPI.VoE_GetCodecs();
        spVoiceCodecType = (Spinner) findViewById(R.id.spVoiceCodecType);
        spVoiceCodecType.setOnItemSelectedListener(this);
        spVoiceCodecType.setAdapter(new SpinnerAdapter(this,
                        R.layout.row,
                        mVoiceCodecsStrings));
        spVoiceCodecType.setSelection(1); // amr 6

        RadioGroup radioGroup = (RadioGroup) findViewById(R.id.radio_group1);
        radioGroup.clearCheck();
       
        if (renderType == RenderType.OPENGL) {
            radioGroup.check(R.id.radio_opengl);
        } else if (renderType == RenderType.SURFACE) {
            radioGroup.check(R.id.radio_surface);
        } else if (renderType == RenderType.MEDIACODEC) {
            radioGroup.check(R.id.radio_mediacodec);
        }
        
        remoteIp = LOOPBACK_IP;
        etRemoteIp = (EditText) findViewById(R.id.etRemoteIp);
        etRemoteIp.setText(LOOPBACK_IP);
        etRemoteIp.setOnClickListener(this);
        
        etRemotePort = (EditText)findViewById(R.id.etRemotePort);
        etRemotePort.setText("6666");
        etRemotePort.setOnClickListener(this);
        
        cbVoice = (CheckBox) findViewById(R.id.cbVoice);
        cbVoice.setChecked(enableVoice);
        cbEnableSpeaker = (CheckBox) findViewById(R.id.cbSpeaker);
        cbEnableSpeaker.setChecked(enableSpeaker);
        cbEnableAGC = (CheckBox) findViewById(R.id.cbAutoGainControl);
        cbEnableAGC.setChecked(enableAGC);
        cbEnableAECM = (CheckBox) findViewById(R.id.cbAECM);
        cbEnableAECM.setChecked(enableAECM);
        cbEnableNS = (CheckBox) findViewById(R.id.cbNoiseSuppression);
        cbEnableNS.setChecked(enableNS);
        
        cbEnableSpeaker.setOnClickListener(this);
        cbEnableAECM.setOnClickListener(this);
        cbEnableAGC.setOnClickListener(this);
        cbEnableNS.setOnClickListener(this);

        readSettings();
        //numCores = 4;
        if(numCores <= 2)
        {
          codecSizeWidth = 320;//320
          codecSizeHeight = 240;//240
        }
        else 
        {
          codecSizeWidth = 640;//640;
          codecSizeHeight = 480;//480;
        }
    }

    private String getRemoteIPString() {
        return etRemoteIp.getText().toString();
    }

    private void startCapture() {

    	 int ret = 0;
    	 Log.e("kxw", "startCapture:"+ret);
    	 
    	 voiceCodecType = spVoiceCodecType.getSelectedItemPosition();
         enableVoice  = cbVoice.isChecked();
         enableSpeaker  = cbEnableSpeaker.isChecked();
         enableAGC  = cbEnableAGC.isChecked();
         enableAECM  = cbEnableAECM.isChecked();
         enableNS  = cbEnableNS.isChecked();
    	 
//	 if (enableVoice) {        
        startVoiceEngine();
//     }
	 
	 if (0 != vieAndroidAPI.VoE_GetSendCodec(voiceChannel)) {
         Log.d(TAG, "VoE get send codec failed");
     }   	 
         /*
	// camera and preview surface
      svLocal = ViERenderer.CreateLocalRenderer(this);
	  //svLocal = testView.getSurface();
      //svLocal.setRotation(90);
     // channel = vieAndroidAPI.CreateChannel(voiceChannel);
      channel2 = vieAndroidAPI.CreateChannel(voiceChannel);
      Log.e("kxw", "Create channel ok: "+channel+":"+channel2);
      currentCameraOrientation = vieAndroidAPI.GetCameraOrientation(usingFrontCamera ? 1 : 0);
      int camId = vieAndroidAPI.StartCamera(channel, usingFrontCamera ? 1 : 0);
      viERunning = true;
      if (camId > 0) {
              cameraId = camId;
              //int neededRotation = getCameraOrientation(currentCameraOrientation);
              vieAndroidAPI.SetRotation(cameraId, curOrientation);
       } 
     if (mLlLocalSurface != null) {
     	 mLlLocalSurface.addView(svLocal);
    }
    */
    }  
    private void startSend() {
    	  Log.e("kxw", "startSend"+"type:"+codecType);
    	  int ret = 0;
    	  //ret = vieAndroidAPI.RegisterSendTransport(true, false);//external transport
    	  ret = vieAndroidAPI.RegisterSendTransport(false, false);
    	  codecSizeWidth = 1920;
    	  codecSizeHeight = 1080;
    	  ret = vieAndroidAPI.SetSendDestination(channel,5555,getRemoteIPString());
    	  ret = vieAndroidAPI.SetSendCodec(channel, codecType, INIT_BITRATE,
                  codecSizeWidth, codecSizeHeight, SEND_CODEC_FRAMERATE);
          ret = vieAndroidAPI.StartSend(channel);
          ret = vieAndroidAPI.SetCallback(channel, this);
          //handler.postDelayed(statViewCallback,1000);
          statView();
    }
    private void startPlayFlv(){
    	Log.i("yyf","startPlayFlv");

    	int ret = 0;    	 
        if (renderType == RenderType.OPENGL) {
            Log.v(TAG, "Create OpenGL Render");
            remoteSurfaceView = ViERenderer.CreateRenderer(this, true);
        } else if (renderType == RenderType.SURFACE) {
            Log.v(TAG, "Create SurfaceView Render");
            remoteSurfaceView = ViERenderer.CreateRenderer(this, false);
        } else if (renderType == RenderType.MEDIACODEC) {
            Log.v(TAG, "Create MediaCodec Decoder/Renderer");
            remoteSurfaceView = new SurfaceView(this);
        }
        
        if (mLlRemoteSurface != null) {
            mLlRemoteSurface.addView(remoteSurfaceView);
        }
        
       
        ret = vieAndroidAPI.StartPlayFlvFile(remoteSurfaceView);
    	/*Log.i("kmm","change 1.9 & 1.4");
    	if(codecType == 3)
    	{
    		codecType = 1;
      	  	codecSizeWidth = 640;
      	  	codecSizeHeight = 480;
    	}
    	else if(codecType ==1)
    	{
    		codecType = 3;
    		codecSizeWidth = 640;
      	  	codecSizeHeight = 360;
    	}
  	  ret = vieAndroidAPI.SetSendCodec(channel, codecType, INIT_BITRATE,
              codecSizeWidth, codecSizeHeight, SEND_CODEC_FRAMERATE);
  	  
  	  ret = vieAndroidAPI.SetReceiveCodec(channel2,
              codecType,
              INIT_BITRATE,
              codecSizeWidth,
              codecSizeHeight,
              RECEIVE_CODEC_FRAMERATE);
      
	  ret = vieAndroidAPI.SetReceiveCodec(channel,
              codecType,
              INIT_BITRATE,
              codecSizeWidth,
              codecSizeHeight,
              RECEIVE_CODEC_FRAMERATE);*/
    
    }
    private void startReceive() {
    	  Log.e("kxw", "startReceive");
    	  int ret = 0;
 
              if (renderType == RenderType.OPENGL) {
                  Log.v(TAG, "Create OpenGL Render");
                  remoteSurfaceView = ViERenderer.CreateRenderer(this, true);
                  remoteSurfaceView2 = ViERenderer.CreateRenderer(this, true);
              } else if (renderType == RenderType.SURFACE) {
                  Log.v(TAG, "Create SurfaceView Render");
                  remoteSurfaceView = ViERenderer.CreateRenderer(this, false);
                  remoteSurfaceView2 = ViERenderer.CreateRenderer(this, false);
              } else if (renderType == RenderType.MEDIACODEC) {
                  Log.v(TAG, "Create MediaCodec Decoder/Renderer");
                  remoteSurfaceView = new SurfaceView(this);
              }

              if (mLlRemoteSurface != null) {
                  mLlRemoteSurface.addView(remoteSurfaceView);
              }
              if (mLlRemoteSurface2 != null) {
                  mLlRemoteSurface2.addView(remoteSurfaceView2);
              }

              if (renderType == RenderType.MEDIACODEC) {
                  ret = vieAndroidAPI.SetExternalMediaCodecDecoderRenderer(
                          channel, remoteSurfaceView);
              } else {
                  ret = vieAndroidAPI.AddRemoteRenderer(channel, remoteSurfaceView);
                  //ret = vieAndroidAPI.AddRemoteRenderer(channel2, remoteSurfaceView2);
              }

              
              codecSizeWidth = 640;
        	  codecSizeHeight = 360;
        	  
              ret = vieAndroidAPI.SetReceiveCodec(channel2,
                      codecType,
                      INIT_BITRATE,
                      codecSizeWidth,
                      codecSizeHeight,
                      RECEIVE_CODEC_FRAMERATE);
              Log.e("kxw", "StartRender");
              ret = vieAndroidAPI.StartRender(channel2);
              ret = vieAndroidAPI.SetRenderType(channel2, 1);
              
              ret = vieAndroidAPI.SetLocalReceiver(channel2,
                      5555);
              
              Log.e("kxw", "StartReceive");
              ret = vieAndroidAPI.StartReceive(channel2);
    	  ret = vieAndroidAPI.EnablePLI(channel2, true);
    	  /*
    	  ret = vieAndroidAPI.SetReceiveCodec(channel,
                  codecType,
                  INIT_BITRATE,
                  codecSizeWidth,
                  codecSizeHeight,
                  RECEIVE_CODEC_FRAMERATE);
          Log.e("kxw", "StartRender");
          ret = vieAndroidAPI.StartRender(channel);
          ret = vieAndroidAPI.SetRenderType(channel, 1);
          
          ret = vieAndroidAPI.SetLocalReceiver(channel,
                  6666);
          
          Log.e("kxw", "StartReceiveeeee");
          ret = vieAndroidAPI.StartReceive(channel);
	  ret = vieAndroidAPI.EnablePLI(channel, true);
    	  
    	  
    	  
          //ret = vieAndroidAPI.EnableNACK(channel, enableNack);
*/
    }

    private void fullScreen1() {
  
        /*findViewById(R.id.llfullScreen).setVisibility(View.VISIBLE);*/
     	findViewById(R.id.btStartCapture).setVisibility(View.GONE);
    	findViewById(R.id.btStartSend).setVisibility(View.GONE);
    	findViewById(R.id.btStartReceive).setVisibility(View.GONE);
    	findViewById(R.id.btStartPlayFlv).setVisibility(View.GONE);
    	findViewById(R.id.btSwitchCamera).setVisibility(View.GONE);
    	findViewById(R.id.btFullScreen).setVisibility(View.GONE);
    	findViewById(R.id.btExit).setVisibility(View.GONE);
    	//findViewById(R.id.statView).setVisibility(View.GONE);
    	//findViewById(R.id.llLocalView).setVisibility(View.GONE);
    	
    	findViewById(R.id.llRemoteView).setLayoutParams(
    			new LinearLayout.LayoutParams(LinearLayout.LayoutParams.FILL_PARENT,LinearLayout.LayoutParams.FILL_PARENT));
    
    	//findViewById(R.id.llRemoteView).setLayoutParams(
    	//		new LinearLayout.LayoutParams(metrics.widthPixels, 320*metrics.widthPixels/240));
    
    }
    
    private void fullScreen() {
	    Log.e("kxw", "capture rotate:");
	    int newRotation = 180;//getCameraOrientation(currentCameraOrientation);
	    if (viERunning) {
	        vieAndroidAPI.SetRotation(cameraId, newRotation);
	    }
    }
    
    private void stopVoiceEngine() {
        // Stop send
        if (0 != vieAndroidAPI.VoE_StopSend(voiceChannel)) {
            Log.d(TAG, "VoE stop send failed");
        }

        // Stop listen
        if (0 != vieAndroidAPI.VoE_StopListen(voiceChannel)) {
            Log.d(TAG, "VoE stop listen failed");
        }

        // Stop playout
        if (0 != vieAndroidAPI.VoE_StopPlayout(voiceChannel)) {
            Log.d(TAG, "VoE stop playout failed");
        }

        if (0 != vieAndroidAPI.VoE_DeleteChannel(voiceChannel)) {
            Log.d(TAG, "VoE delete channel failed");
        }
        voiceChannel = -1;

        // Terminate
        if (0 != vieAndroidAPI.VoE_Terminate()) {
            Log.d(TAG, "VoE terminate failed");
        }
    }
    
    private int setupVoE() {
        // Create VoiceEngine
        // Error logging is done in native API wrapper
        vieAndroidAPI.VoE_Create(getApplicationContext(), callBack);

        // Initialize
        if (0 != vieAndroidAPI.VoE_Init(enableTrace)) {
            Log.d(TAG, "VoE init failed");
            return -1;
        }
        
        // VoiceObserver
        if (0 != vieAndroidAPI.VoE_RegisterVoiceEngineObserver()) {
            Log.d(TAG, "VoE RegisterVoiceEngineObserver failed");
            return -1;
        }
        
        //combodata _InputScaler:4 _AudioManagerMode:4 _AudioStreamType:4 _recAudioSource:4
		//_recAudioSource 1 is AudioSource.MIC which is our default
		//_AudioManagerMode 0 is MODE_NORMAL, which is our default, 2 is MODE_IN_CALL, 3 is MODE_IN_COMMUNICATION
		//_AudioStreamType 0 is STREAM_VOICE_CALL,  which is our default, 3 is STREAM_MUSIC
		//_InputScaler  0 is not scaler ,1 is *2, 4 is not scaler, 5 is /2
		//_InputScaler >>12
		//_AudioManagerMode >>8&0xf
		//_AudioStreamType >>4&0xf
		//_recAudioSource &0xf
        if (0 != vieAndroidAPI.VoE_SetRecordingDevice(0x0207)) {
            Log.d(TAG, "VoE SetRecordingDevice failed");
            return -1;
        }

        // Create channel
        voiceChannel = vieAndroidAPI.VoE_CreateChannel();
        if (0 != voiceChannel) {
            Log.d(TAG, "VoE create channel failed");
            return -1;
        }

        // Suggest to use the voice call audio stream for hardware volume controls
        setVolumeControlStream(AudioManager.STREAM_VOICE_CALL);
        return 0;
    }
    
    private int startVoiceEngine() {
    	voiceSendPort = Integer.parseInt(etRemotePort.getText().toString());
        // Set local receiver
        if (0 != vieAndroidAPI.VoE_SetLocalReceiver(voiceChannel,
                        6666)) {
            Log.d(TAG, "VoE set local receiver failed");
        }

        if (0 != vieAndroidAPI.VoE_StartListen(voiceChannel)) {
            Log.d(TAG, "VoE start listen failed");
        }

        // Route audio
        routeAudio(enableSpeaker);

        // set volume to default value
        if (0 != vieAndroidAPI.VoE_SetSpeakerVolume(204)) {
            Log.d(TAG, "VoE set speaker volume failed");
        }

        // Start playout
        if (0 != vieAndroidAPI.VoE_StartPlayout(voiceChannel)) {
            Log.d(TAG, "VoE start playout failed");
        }

        if (0 != vieAndroidAPI.VoE_SetSendDestination(voiceChannel,
        		voiceSendPort,
                                                      getRemoteIPString())) {
            Log.d(TAG, "VoE set send  destination failed");
        }

        if (0 != vieAndroidAPI.VoE_SetSendCodec(voiceChannel, voiceCodecType)) {
            Log.d(TAG, "VoE set send codec failed");
        }

        if (0 != vieAndroidAPI.VoE_SetECStatus(enableAECM)) {
            Log.d(TAG, "VoE set EC Status failed");
        }

        if (0 != vieAndroidAPI.VoE_SetAGCStatus(enableAGC)) {
            Log.d(TAG, "VoE set AGC Status failed");
        }

        if (0 != vieAndroidAPI.VoE_SetNSStatus(enableNS)) {
            Log.d(TAG, "VoE set NS Status failed");
        }

        if (0 != vieAndroidAPI.VoE_StartSend(voiceChannel)) {
            Log.d(TAG, "VoE start send failed");
        }

        voERunning = true;
        return 0;
    }
    
    private void routeAudio(boolean enableSpeaker) {
        if (0 != vieAndroidAPI.VoE_SetLoudspeakerStatus(enableSpeaker)) {
            Log.d(TAG, "VoE set louspeaker status failed");
        }
    }
    
    private int startVoiceEngineSpeex() {
    	if (voiceChannel >= 0)
    	{    		
    		//--------
    		if (0 != vieAndroidAPI.VoE_SetECStatus(true)) {
                Log.d(TAG, "VoE set EC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetAGCStatus(false)) {
                Log.d(TAG, "VoE set AGC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetNSStatus(true)) {
                Log.d(TAG, "VoE set NS Status failed");
            }
            //--------
            voiceCodecType = 9;
            if (0 != vieAndroidAPI.VoE_SetSendCodec(voiceChannel, voiceCodecType)) {
                Log.d(TAG, "VoE set send codec failed");
            }            
            if (0 != vieAndroidAPI.VoE_RegisterExternalTransport(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }            
            if (0 != vieAndroidAPI.VoE_StartSend(voiceChannel)) {
                Log.d(TAG, "VoE start send failed");
            }            
            if (0 != vieAndroidAPI.VoE_SetSpeakerVolume(204)) {
                Log.d(TAG, "VoE set speaker volume failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_SetVADStatus(voiceChannel, true, 3, false)) {
                Log.d(TAG, "VoE set VAD Status failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_StartListen(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }       
            if (0 != vieAndroidAPI.VoE_StartPlayout(voiceChannel)) {
                Log.d(TAG, "VoE start playout failed");
            }
          
            voERunning = true;    		
    	}
        return 0;
    }
    
    private int startVoiceEngineAmr() {
    	if (voiceChannel >= 0)
    	{    		
    		//--------
    		if (0 != vieAndroidAPI.VoE_SetECStatus(true)) {
                Log.d(TAG, "VoE set EC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetAGCStatus(false)) {
                Log.d(TAG, "VoE set AGC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetNSStatus(true)) {
                Log.d(TAG, "VoE set NS Status failed");
            }
            //--------
            voiceCodecType = 6;
            if (0 != vieAndroidAPI.VoE_SetSendCodec(voiceChannel, voiceCodecType)) {
                Log.d(TAG, "VoE set send codec failed");
            }            
            if (0 != vieAndroidAPI.VoE_RegisterExternalTransport(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }            
            if (0 != vieAndroidAPI.VoE_StartSend(voiceChannel)) {
                Log.d(TAG, "VoE start send failed");
            }            
            if (0 != vieAndroidAPI.VoE_SetSpeakerVolume(204)) {
                Log.d(TAG, "VoE set speaker volume failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_SetVADStatus(voiceChannel, true, 3, false)) {
                Log.d(TAG, "VoE set VAD Status failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_StartListen(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }       
            if (0 != vieAndroidAPI.VoE_StartPlayout(voiceChannel)) {
                Log.d(TAG, "VoE start playout failed");
            }
          
            voERunning = true;    		
    	}
        return 0;
    }

    private int startVoiceEngineOpus() {
    	if (voiceChannel >= 0)
    	{    		
    		//--------
    		if (0 != vieAndroidAPI.VoE_SetECStatus(false)) {
                Log.d(TAG, "VoE set EC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetAGCStatus(false)) {
                Log.d(TAG, "VoE set AGC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetNSStatus(true)) {
                Log.d(TAG, "VoE set NS Status failed");
            }
            //--------
            voiceCodecType = 7;
            if (0 != vieAndroidAPI.VoE_SetSendCodec(voiceChannel, voiceCodecType)) {
                Log.d(TAG, "VoE set send codec failed");
            }            
            if (0 != vieAndroidAPI.VoE_RegisterExternalTransport(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }            
            if (0 != vieAndroidAPI.VoE_StartSend(voiceChannel)) {
                Log.d(TAG, "VoE start send failed");
            }            
            if (0 != vieAndroidAPI.VoE_SetSpeakerVolume(204)) {
                Log.d(TAG, "VoE set speaker volume failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_SetVADStatus(voiceChannel, true, 3, false)) {
                Log.d(TAG, "VoE set VAD Status failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_StartListen(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }       
            if (0 != vieAndroidAPI.VoE_StartPlayout(voiceChannel)) {
                Log.d(TAG, "VoE start playout failed");
            }
          
            voERunning = true;    		
    	}
        return 0;
    }
    
    private int startVoiceEngineIlbc() {
    	if (voiceChannel >= 0)
    	{    		
    		//--------
    		if (0 != vieAndroidAPI.VoE_SetECStatus(true)) {
                Log.d(TAG, "VoE set EC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetAGCStatus(false)) {
                Log.d(TAG, "VoE set AGC Status failed");
            }
            if (0 != vieAndroidAPI.VoE_SetNSStatus(true)) {
                Log.d(TAG, "VoE set NS Status failed");
            }
            //--------
            voiceCodecType = 5;
            if (0 != vieAndroidAPI.VoE_SetSendCodec(voiceChannel, voiceCodecType)) {
                Log.d(TAG, "VoE set send codec failed");
            }            
            if (0 != vieAndroidAPI.VoE_RegisterExternalTransport(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }            
            if (0 != vieAndroidAPI.VoE_StartSend(voiceChannel)) {
                Log.d(TAG, "VoE start send failed");
            }            
            if (0 != vieAndroidAPI.VoE_SetSpeakerVolume(204)) {
                Log.d(TAG, "VoE set speaker volume failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_SetVADStatus(voiceChannel, true, 3, false)) {
                Log.d(TAG, "VoE set VAD Status failed");
            }
            //--------
            if (0 != vieAndroidAPI.VoE_StartListen(voiceChannel)) {
                Log.d(TAG, "VoE start listen failed");
            }       
            if (0 != vieAndroidAPI.VoE_StartPlayout(voiceChannel)) {
                Log.d(TAG, "VoE start playout failed");
            }
          
            voERunning = true;    		
    	}
        return 0;
    }
    
    private int stopAudio() {
    	// Stop send
        if (0 != vieAndroidAPI.VoE_StopSend(voiceChannel)) {
            Log.d(TAG, "VoE stop send failed");
        }

        // Stop listen
        if (0 != vieAndroidAPI.VoE_StopListen(voiceChannel)) {
            Log.d(TAG, "VoE stop listen failed");
        }

        // Stop playout
        if (0 != vieAndroidAPI.VoE_StopPlayout(voiceChannel)) {
            Log.d(TAG, "VoE stop playout failed");
        }
        
        return 0;
    }

    public void onClick(View arg0) {
        switch (arg0.getId()) {
            case R.id.btSwitchCamera:
            	Log.e("kxw", "switch camera");
                if (usingFrontCamera) {
                    btSwitchCamera.setText(R.string.frontCamera);
                } else {
                    btSwitchCamera.setText(R.string.backCamera);
                }
                usingFrontCamera = !usingFrontCamera;

                if (viERunning) {
                    currentCameraOrientation =
                            vieAndroidAPI.GetCameraOrientation(usingFrontCamera ? 1 : 0);
                    vieAndroidAPI.StopCamera(cameraId);
                    mLlLocalSurface.removeView(svLocal);

                   cameraId = vieAndroidAPI.StartCamera(channel, usingFrontCamera ? 1 : 0);
                    mLlLocalSurface.addView(svLocal);
                    int neededRotation = getCameraOrientation(currentCameraOrientation);
                    vieAndroidAPI.SetRotation(cameraId, curOrientation);
                }
                break;
            case R.id.btStartCapture:
            	startCapture();
            	break;
            case R.id.btStartSend:
                startSend();
            	break;
            case R.id.btStartReceive:
                startReceive();
            	break;
            case R.id.btStartPlayFlv:
                startPlayFlv();
            	break;
            case R.id.btSnapLocal:
                SnapLocal();
            	break;
            case R.id.btSnapRemote:
                SnapRemote();
            	break;
            case R.id.btSetCaptureFlashlight:
            	usingFlashlight = !usingFlashlight;            	
            	SetCaptureFlashlight(cameraId, usingFlashlight);
            	break;
            case R.id.btFullScreen:
                fullScreen();
            	break;
            case R.id.btExit:
                stopAll();
                finish();
                break;
            case R.id.etRemoteIp:
                remoteIp = etRemoteIp.getText().toString();
                break;
            case R.id.radio_surface:
                renderType = RenderType.SURFACE;
                break;
            case R.id.radio_opengl:
                renderType = RenderType.OPENGL;
                break;
            case R.id.radio_mediacodec:
                renderType = RenderType.MEDIACODEC;
                break;
            case R.id.cbSpeaker:
                enableSpeaker = cbEnableSpeaker.isChecked();
                if (voERunning) {
                    routeAudio(enableSpeaker);
                }
                break;
            case R.id.cbAutoGainControl:
                enableAGC = cbEnableAGC.isChecked();
                if (voERunning) {
                    vieAndroidAPI.VoE_SetAGCStatus(enableAGC);
                }
                break;
            case R.id.cbNoiseSuppression:
                enableNS = cbEnableNS.isChecked();
                if (voERunning) {
                    vieAndroidAPI.VoE_SetNSStatus(enableNS);
                }
                break;
            case R.id.cbAECM:
                enableAECM = cbEnableAECM.isChecked();
                if (voERunning) {
                    vieAndroidAPI.VoE_SetECStatus(enableAECM);
                }
                break;
            case R.id.btAudioSpeex:
            	startVoiceEngineSpeex();
            	break;
            case R.id.btAudioAmr:
            	startVoiceEngineAmr();
            	break;
            case R.id.btAudioOpus:
            	startVoiceEngineOpus();
            	break;
            case R.id.btAudioIlbc:
            	startVoiceEngineIlbc();
            	break;
            case R.id.btAudioStop:
            	stopAudio();
            	break;
        }
    }
    
    public boolean onTouchEvent(MotionEvent event)
    {
    	switch(event.getAction())
    	{
    	case MotionEvent.ACTION_DOWN:
    		
    		
    		findViewById(R.id.llRemoteView).setLayoutParams(
    				layout);

    		if(findViewById(R.id.llRemoteView).getVisibility() == View.VISIBLE)
    		{
    			touchCount++;
    			if(1 == touchCount)
    			{
    				firstClick = (int)System.currentTimeMillis();
    			}
    			if(2 == touchCount)
    			{
    				secondClick = (int)System.currentTimeMillis();
    				if ((secondClick - firstClick) < 700)
    				{
    					//findViewById(R.id.llfullScreen).setVisibility(View.GONE);
    					findViewById(R.id.btStartCapture).setVisibility(View.VISIBLE);
    			    	findViewById(R.id.btStartSend).setVisibility(View.VISIBLE);
    			    	findViewById(R.id.btStartReceive).setVisibility(View.VISIBLE);
    			    	findViewById(R.id.btStartPlayFlv).setVisibility(View.VISIBLE);
    			    	findViewById(R.id.btSwitchCamera).setVisibility(View.VISIBLE);
    			    	findViewById(R.id.btFullScreen).setVisibility(View.VISIBLE);
    			    	findViewById(R.id.btExit).setVisibility(View.VISIBLE);
    					findViewById(R.id.llLocalView).setVisibility(View.VISIBLE);
    				}
    				touchCount = 0;
    			}
    		}
    		break;
    	case MotionEvent.ACTION_UP:
    		break;
    	default:
    		break;
    	}
    	//return super.onTouchEvent(event);
    	return true;
    }

    private void readSettings() {
        codecType = spCodecType.getSelectedItemPosition();
        codecType = 3;
        String sCodecSize = spCodecSize.getSelectedItem().toString();
        String[] aCodecSize = sCodecSize.split("x");
        codecSizeWidth = Integer.parseInt(aCodecSize[0]);
        codecSizeHeight = Integer.parseInt(aCodecSize[1]);
    }

    public void onItemSelected(AdapterView<?> adapterView, View view,
            int position, long id) {
        if ((adapterView == spCodecType || adapterView == spCodecSize) &&
                viERunning) {
            readSettings();
            // change the codectype

                if (0 != vieAndroidAPI.SetReceiveCodec(channel, codecType,
                                INIT_BITRATE, codecSizeWidth,
                                codecSizeHeight,
                                RECEIVE_CODEC_FRAMERATE)) {
                    Log.d(TAG, "ViE set receive codec failed");
                }
                if (0 != vieAndroidAPI.SetSendCodec(channel, codecType,
                                INIT_BITRATE, codecSizeWidth, codecSizeHeight,
                                SEND_CODEC_FRAMERATE)) {
                    Log.d(TAG, "ViE set send codec failed");
                }

        } 
    }

    public void onNothingSelected(AdapterView<?> arg0) {
        Log.d(TAG, "No setting selected");
    }

    public int updateStats(int inFrameRateI, int inBitRateI,
            int inPacketLoss, int inFrameRateO, int inBitRateO) {
        frameRateI = inFrameRateI;
        bitRateI = inBitRateI;
        packetLoss = inPacketLoss;
        frameRateO = inFrameRateO;
        bitRateO = inBitRateO;
        return 0;
    }
    
    private void setFullScreen()
    {
    	getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
    			             WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }
    private void quitFullScreen()
    {
         final WindowManager.LayoutParams attrs = getWindow().getAttributes();
         attrs.flags &= (~WindowManager.LayoutParams.FLAG_FULLSCREEN);
         getWindow().setAttributes(attrs);
         getWindow().clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
    }
    
    private void SnapRemote() {
	    Log.e("yyf", "snapRemote:");	  
	    if (viERunning) {
	        vieAndroidAPI.SnapRemote(channel, "/sdcard/remote1.jpg");
	        //vieAndroidAPI.SnapRemote(channel2, "/sdcard/remote2.jpg");
	    }
    }
    
    private void SnapLocal() {
	    Log.e("yyf", "snapLocal:");	  
	    if (viERunning) {
	        vieAndroidAPI.SnapLocal(cameraId, "/sdcard/local.jpg");
	    }
    }
    
    private void SetCaptureFlashlight(int id, boolean on){
    	Log.e("yyf", "SetCaptureFlashlight:");	  
	    if (viERunning) {
	        vieAndroidAPI.SetCaptureFlashlight(cameraId, on);
	    }
    }
}

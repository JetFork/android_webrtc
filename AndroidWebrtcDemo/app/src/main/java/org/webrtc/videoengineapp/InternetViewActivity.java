package org.webrtc.videoengineapp;

import org.webrtc.videoengine.ViERenderer;
import org.webrtc.videoengineapp.WebRTCDemo.RenderType;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class InternetViewActivity extends Activity implements IViEAndroidCallback, View.OnClickListener
{
    private static final int DEFAULT_FR = 25;
    private static final int DEFAULT_BR = 200;
    private static final String LOOPBACK_IP = "127.0.0.1";
    private Button btnStart = null, btnSend=null, btnRecv = null;
    private LinearLayout internetRemoteView = null;
    private LinearLayout internetLocalView = null;
    private ViEAndroidJavaAPI svcAPI = null;
    private SurfaceView svcLocal = null;
    // local renderer and camera
    private SurfaceView svcRemote = null;
    private int voiceChannel = -1;
    private int recvChannel = -1;
    private int sendChannel = -1;
    private int camDirect = 1;
    private boolean isRunning = false;
    private int cameraID = -1;
    private TextView tview = null;
    private int frameRateI;
    private int bitRateI;
    private int packetLoss;
    private int frameRateO;
    private int bitRateO;
    private int width = 640, height = 480;
    private int codecType = 1;
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_internet_view);
        internetRemoteView = (LinearLayout) findViewById(R.id.internetRemote);
        internetLocalView = (LinearLayout) findViewById(R.id.internetLocal);
        btnStart = (Button)findViewById(R.id.internetButtonStart);
        btnStart.setOnClickListener(this);
        
        btnSend= (Button)findViewById(R.id.internetButtonSend);
        btnSend.setOnClickListener(this);
        
        btnRecv= (Button)findViewById(R.id.internetButtonReceive);
        btnRecv.setOnClickListener(this);
        tview = (TextView)findViewById(R.id.statSVC);
        tview.setText("stats here");
        init();
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
	 private Handler handler = new Handler();
	    private Runnable statViewCallback = new Runnable() {
	            public void run() {
	            	statView();
	            }
	        };
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
	public void  init()
	{
        Log.e("kxw", "init start");
        if (null == svcAPI) {
        	svcAPI = new ViEAndroidJavaAPI(this);
        }
        if (0 > svcAPI.GetVideoEngine() ||
                0 > svcAPI.Init(true)) {
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

       
	}
	private void startCapture() {

   	 int ret = 0;
   	 Log.e("kxw", "startCapture:"+ret);
        // camera and preview surface
   	svcLocal = ViERenderer.CreateLocalRenderer(this);
     sendChannel = svcAPI.CreateChannel(voiceChannel);
    // currentCameraOrientation = vieAndroidAPI.GetCameraOrientation(usingFrontCamera ? 1 : 0);
      cameraID = svcAPI.StartCamera(sendChannel, camDirect);
     isRunning = true;
    /* if (camId > 0) {
             cameraId = camId;
             int neededRotation = getCameraOrientation(currentCameraOrientation);
             vieAndroidAPI.SetRotation(cameraId, neededRotation);
      } */
     if(cameraID > 0){
    	 svcAPI.SetRotation(cameraID, 270);
     }
    if (internetLocalView != null) {
    	internetLocalView.addView(svcLocal);
   }
   }  
	
	
	
	private void startSend() {
  	  Log.e("kxw", "startSend");
 // 	svcAPI.SetNotify();
  	  int ret = 0;
  	  ret = svcAPI.RegisterSendTransport(true, true);
  	  ret = svcAPI.SetSendDestination(sendChannel,5555,LOOPBACK_IP);
  	  ret = svcAPI.SetSendCodec(sendChannel, codecType, DEFAULT_BR,
                width, height, DEFAULT_FR);
        ret = svcAPI.StartSend(sendChannel);
        ret = svcAPI.SetCallback(sendChannel, this);
        //handler.postDelayed(statViewCallback,1000);
        statView();
  }
  private void startReceive() {
  	  Log.e("kxw", "startReceive");
  	  int ret = 0;
  	recvChannel = sendChannel;//= svcAPI.CreateChannel(-1);
          //  if (renderType == RenderType.OPENGL) {
                Log.e("kxw", "Create OpenGL Render");
                svcRemote = ViERenderer.CreateRenderer(this, true);
          /*  } else if (renderType == RenderType.SURFACE) {
                Log.v(TAG, "Create SurfaceView Render");
                remoteSurfaceView = ViERenderer.CreateRenderer(this, false);
            } else if (renderType == RenderType.MEDIACODEC) {
                Log.v(TAG, "Create MediaCodec Decoder/Renderer");
                remoteSurfaceView = new SurfaceView(this);
            }*/

            if (internetRemoteView != null) {
            	internetRemoteView.addView(svcRemote);
            }

          /*  if (renderType == RenderType.MEDIACODEC) {
                ret = vieAndroidAPI.SetExternalMediaCodecDecoderRenderer(
                        channel, remoteSurfaceView);
            } else */{
                ret = svcAPI.AddRemoteRenderer(recvChannel, svcRemote);
            }

            ret = svcAPI.SetReceiveCodec(recvChannel,
                    codecType,
                    DEFAULT_BR,
                    width,
                    height,
                    DEFAULT_FR);
            Log.e("kxw", "StartRender");
            ret = svcAPI.StartRender(recvChannel);
            ret = svcAPI.SetLocalReceiver(recvChannel,
                    5555);
            
            Log.e("kxw", "StartReceive");
            ret = svcAPI.StartReceive(recvChannel);
  	  ret = svcAPI.EnablePLI(recvChannel, true);
        //ret = vieAndroidAPI.EnableNACK(channel, enableNack);

  }
	
	
    public void onClick(View arg0) {
        switch (arg0.getId()) {
            case R.id.internetButtonStart:
            	Log.e("kxw", "internetButtonStart");
            	startCapture();
                break;
            case R.id.internetButtonSend:
            	Log.e("kxw", "internetButtonSend");
            	startSend();
                break;
            case R.id.internetButtonReceive:
            	Log.e("kxw", "internetButtonReceive");
            	startReceive();
                break;
        }
    }
	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.internet_view, menu);
		return true;
	}
	  @Override
	    public void onDestroy() {
	        Log.e("kxw", "onDestroy");
	        handler = null;//.removeCallbacks(statViewCallback);
	        //unregisterReceiver(receiver);
	        stopAll();
	        super.onDestroy();
	    }
	 private void stopAll() {
	        Log.d("kxw", "stopAll");

	        if (svcAPI != null) {
	          //  if (viERunning) {
	               // viERunning = false;
	        	   int channel = sendChannel;
	                svcAPI.StopRender(channel);
	                svcAPI.StopReceive(channel);
	                svcAPI.StopSend(channel);
	                svcAPI.RemoveRemoteRenderer(channel);
	                svcAPI.StopCamera(cameraID);
	                svcAPI.Terminate();
	                internetRemoteView.removeView(svcRemote);
	                internetLocalView.removeView(svcLocal);
	                internetRemoteView = null;
	                //svLocal = null;
	                svcAPI = null;
	            //}
	           
	        }
	    }
	 
	 public static int jniNotify(int num)
	 {
		  Log.e("kxw", "get notify:"+num);
		  return num<<1;
	 }
}

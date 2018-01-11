package org.webrtc.videoengineapp;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;

public class LoginLayoutActivity extends Activity implements View.OnClickListener
{
	private Button btnStart = null;
	private EditText txtIP = null, txtGID = null;
	private RadioButton rb1, rb2;
	private RadioGroup rg1;
	
	public enum ViewMode {
        SINGLE_MODE,
        INTERNET_MODE
    }
	private ViewMode vmode = ViewMode.SINGLE_MODE;
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.login_layout);
		
		btnStart = (Button) findViewById(R.id.button_start);
		btnStart.setOnClickListener(this);
		txtIP = (EditText)findViewById(R.id.editText1);
		txtGID = (EditText)findViewById(R.id.editText2);
		rb1 = (RadioButton)findViewById(R.id.radioButton1);
		rb2 = (RadioButton)findViewById(R.id.radioButton2);
		rg1 = (RadioGroup)findViewById(R.id.radio_group_view_mode);
		rg1.check(rb1.getId());
		txtIP.setEnabled(false);
		txtGID.setEnabled(false);
		vmode = ViewMode.SINGLE_MODE;
		rg1.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener(){
			@Override
	           public void onCheckedChanged(RadioGroup group, int checkedId) {
	              if(checkedId == rb1.getId()){
	            	  Log.e("kxw", "single mode");
	            	  txtIP.setEnabled(false);
	            	  txtGID.setEnabled(false);
	            	  vmode = ViewMode.SINGLE_MODE;
	              }else if(checkedId == rb2.getId()){
	            	  Log.e("kxw", "internet mode");
	            	  txtIP.setEnabled(true);
	            	  txtGID.setEnabled(true);
	            	  vmode = ViewMode.INTERNET_MODE;
	              }         
			}
		});
	}
	private void onButtonStartClick()
	{
		
		Intent mainIntent  = null;
		if(vmode == ViewMode.SINGLE_MODE){
			Log.e("kxw", "start single mode");
		mainIntent = new Intent(LoginLayoutActivity.this,  WebRTCDemo.class);
		//String ip = txtIP.getText();
		//Log.e("kxw", txtIP.getText().toString());
        mainIntent.putExtra("org.webrtc.videoengineapp.serverIP",txtIP.getText().toString());  
       mainIntent.putExtra("org.webrtc.videoengineapp.groupID", txtGID.getText().toString());  
        LoginLayoutActivity.this.startActivity(mainIntent);  
		}else{
			Log.e("kxw", "start internet mode");
			mainIntent = new Intent(LoginLayoutActivity.this,  InternetViewActivity.class);
	        //mainIntent.putExtra("org.webrtc.videoengineapp.serverIP",txtIP.getText().toString());  
	       // mainIntent.putExtra("org.webrtc.videoengineapp.groupID", txtGID.getText().toString());  
	        LoginLayoutActivity.this.startActivity(mainIntent);  
		}
	}
	public void onClick(View arg0) {
		Log.e("kxw", "login click");
        switch (arg0.getId()) {
            case R.id.button_start:
            Log.e("kxw", "button_start");
            onButtonStartClick();
            /*case R.id.radioButton1:
            	Log.e("kxw", "single mode");
                break;
            case R.id.radioButton2:
            	Log.e("kxw", "internet mode");
                break;*/
            case R.id.btStartCapture:
            	break;
        }
    }

/*	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.login_layout, menu);
		return true;
	}
*/
}

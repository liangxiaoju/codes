package com.example.keytest;

import android.os.Bundle;
import android.app.Activity;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	private TextView mTextView;

	@Override
	public void onAttachedToWindow() {
		// TODO Auto-generated method stub
		//this.getWindow().setType(WindowManager.LayoutParams.TYPE_KEYGUARD);
		super.onAttachedToWindow();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		mTextView = (TextView) this.findViewById(R.id.textview_showkey);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		//mTextView.setText("key=" + KeyEvent.keyCodeToString((event.getKeyCode())) + "(" + String.valueOf(event.getKeyCode()) + ")");
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		// TODO Auto-generated method stub
		mTextView.setText("key=" + KeyEvent.keyCodeToString((event.getKeyCode())) + "(" + String.valueOf(event.getKeyCode()) + ")");
		return true;
	}

	@Override
	public boolean dispatchKeyShortcutEvent(KeyEvent event) {
		// TODO Auto-generated method stub
		return super.dispatchKeyShortcutEvent(event);
	}

	@Override
	public boolean dispatchGenericMotionEvent(MotionEvent event) {
		// TODO Auto-generated method stub
		//mTextView.setText(event.toString());
		mTextView.setText(
				"X=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_X)) + "\n" +
				"Y=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_Y)) + "\n" +
				"Z=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_Z)) + "\n" +
				"RZ=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_RZ)) + "\n" +
				"HAT0X=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_HAT_X)) + "\n" +
				"HAT0Y=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_HAT_Y))
				);
     //if (event.isFromSource(InputDevice.SOURCE_CLASS_JOYSTICK)) {
         if (event.getAction() == MotionEvent.ACTION_MOVE) {
             // process the joystick movement...
        	 	
             return true;
         }
     //}
     //if (event.isFromSource(InputDevice.SOURCE_CLASS_POINTER)) {
         switch (event.getAction()) {
             case MotionEvent.ACTION_HOVER_MOVE:
                 // process the mouse hover movement...
                 return true;
             case MotionEvent.ACTION_SCROLL:
                 // process the scroll wheel movement...
                 return true;
         }
     //}
		return super.dispatchGenericMotionEvent(event);
	}
/*
	@Override
	 public boolean onGenericMotionEvent(MotionEvent event) {
			//mTextView.setText(event.toString());
			mTextView.setText(
					"X=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_X)) + "\n" +
					"Y=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_Y)) + "\n" +
					"Z=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_Z)) + "\n" +
					"RZ=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_RZ)) + "\n" +
					"HAT0X=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_HAT_X)) + "\n" +
					"HAT0Y=" + String.valueOf(event.getAxisValue(MotionEvent.AXIS_HAT_Y))
					);
	     //if (event.isFromSource(InputDevice.SOURCE_CLASS_JOYSTICK)) {
	         if (event.getAction() == MotionEvent.ACTION_MOVE) {
	             // process the joystick movement...
	        	 	
	             return true;
	         }
	     //}
	     //if (event.isFromSource(InputDevice.SOURCE_CLASS_POINTER)) {
	         switch (event.getAction()) {
	             case MotionEvent.ACTION_HOVER_MOVE:
	                 // process the mouse hover movement...
	                 return true;
	             case MotionEvent.ACTION_SCROLL:
	                 // process the scroll wheel movement...
	                 return true;
	         }
	     //}
	     return super.onGenericMotionEvent(event);
	 }
*/
}

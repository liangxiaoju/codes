package com.example.voicerecorder;

import android.app.Service;
import android.content.Intent;
import android.content.Context;
import android.media.MediaRecorder;
import android.os.IBinder;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

public class RecorderService extends Service {
	
	private Recorder mRecorder;
	
	public RecorderService() {
	}

	@Override
	public IBinder onBind(Intent intent) {
        return null;
	}

	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
		
		mRecorder = new Recorder();
		
		TelephonyManager tm = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
		tm.listen(new CallStateListener(), PhoneStateListener.LISTEN_CALL_STATE);
		
		System.out.println("RecorderService start");
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		System.out.println("RecorderService stop");
	}

	public class CallStateListener extends PhoneStateListener {
		String number;

		@Override
		public void onCallStateChanged(int state, String incomingNumber) {
			switch (state) {
			case TelephonyManager.CALL_STATE_IDLE:
				System.out.println("CALL_STATE_IDLE");
				//stopRecorder();
				break;
			case TelephonyManager.CALL_STATE_RINGING:
				System.out.println("CALL_STATE_RINGING" + ": " + incomingNumber);
				number = incomingNumber;
				break;
			case TelephonyManager.CALL_STATE_OFFHOOK:
				System.out.println("CALL_STATE_OFFHOOK");
				//startRecorder();
				break;
			}
		}
		
	}
	
	private void startRecorder() {
		Command cmd = new Command("/system/bin/service call media.audio_policy 1 i32 2147483712 i32 1 s16 voice_call");
		if (cmd.exec())
			System.out.println("OK: enable VOICE_CALL");
		else
			System.out.println("ERR: enable VOICE_CALL");
		mRecorder.setAudioSource(MediaRecorder.AudioSource.VOICE_CALL);
		mRecorder.startRecording();
	}
	
	private void stopRecorder() {
		mRecorder.stopRecording();
	}
}

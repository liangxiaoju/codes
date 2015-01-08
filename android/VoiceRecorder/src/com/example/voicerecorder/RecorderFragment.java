package com.example.voicerecorder;

import java.io.File;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.media.MediaRecorder;

public class RecorderFragment extends Fragment implements
		Button.OnClickListener, Recorder.OnRecorderListener {

	private static final int STATE_IDLE = 0;
	private static final int STATE_RECORDING = 1;
	private static final int STATE_RECORDED = 2;
	private static final int STATE_PLAYING = 3;

	private TextView mTextView;
	private Button mButtonRecord;
	private Button mButtonPlayback;
	private Button mButtonStop;
	private Button mButtonSave;
	private Button mButtonCancel;
	private Recorder mRecorder;
	private OnRecorderListener mRecorderListener;

	public interface OnRecorderListener {
		public void onRecordSaved(File f);
	}

	public static Fragment newInstance() {
		return new RecorderFragment();
	}

	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		try {
			mRecorderListener = (OnRecorderListener) activity;
		} catch (ClassCastException e) {
			throw new ClassCastException(activity.toString()
					+ "must implement OnRecorderListener");
		}
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {

		View rootView = inflater.inflate(R.layout.fragment_recorder, container,
				false);

		mTextView = (TextView) rootView.findViewById(R.id.textview_status);
		mButtonRecord = (Button) rootView.findViewById(R.id.button_record);
		mButtonPlayback = (Button) rootView.findViewById(R.id.button_playback);
		mButtonStop = (Button) rootView.findViewById(R.id.button_stop);
		mButtonSave = (Button) rootView.findViewById(R.id.button_save);
		mButtonCancel = (Button) rootView.findViewById(R.id.button_cancel);

		mButtonRecord.setOnClickListener(this);
		mButtonPlayback.setOnClickListener(this);
		mButtonStop.setOnClickListener(this);
		mButtonSave.setOnClickListener(this);
		mButtonCancel.setOnClickListener(this);

		mRecorder = new Recorder();
		mRecorder.setRecorderListener(this);

		setState(STATE_IDLE);

		// startService(new Intent(this, RecorderService.class));

		return rootView;
	}

	@Override
	public void onResume() {
		super.onResume();
	}

	@Override
	public void onPause() {
		super.onPause();
	}

	private void setState(int state) {
		switch (state) {
		case STATE_IDLE:
			mTextView.setText("Ready");
			mButtonRecord.setVisibility(View.VISIBLE);
			mButtonPlayback.setVisibility(View.GONE);
			mButtonStop.setVisibility(View.GONE);
			mButtonSave.setVisibility(View.GONE);
			mButtonCancel.setVisibility(View.GONE);

			mButtonRecord.setEnabled(true);
			mButtonRecord.setFocusable(true);
			mButtonRecord.requestFocus();
			break;
		case STATE_RECORDING:
			mTextView.setText("Recording");
			mButtonRecord.setVisibility(View.GONE);
			mButtonPlayback.setVisibility(View.GONE);
			mButtonStop.setVisibility(View.VISIBLE);
			mButtonSave.setVisibility(View.GONE);
			mButtonCancel.setVisibility(View.GONE);

			mButtonStop.setEnabled(true);
			mButtonStop.setFocusable(true);
			break;
		case STATE_RECORDED:
			mTextView.setText("Stop");
			mButtonRecord.setVisibility(View.GONE);
			mButtonPlayback.setVisibility(View.VISIBLE);
			mButtonStop.setVisibility(View.GONE);
			mButtonSave.setVisibility(View.VISIBLE);
			mButtonCancel.setVisibility(View.VISIBLE);

			mButtonPlayback.setEnabled(true);
			mButtonPlayback.setFocusable(true);
			mButtonSave.setEnabled(true);
			mButtonSave.setFocusable(true);
			mButtonCancel.setEnabled(true);
			mButtonCancel.setFocusable(true);
			break;
		case STATE_PLAYING:
			mTextView.setText("Playing");
			break;
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub

		switch (v.getId()) {
		case R.id.button_record:
			startRecord();
			break;
		case R.id.button_playback:
			startPlayback();
			break;
		case R.id.button_stop:
			stop();
			break;
		case R.id.button_save:
			save();
			break;
		case R.id.button_cancel:
			cancel();
			break;
		}
	}

	private void startRecord() {
		mRecorder.stop();
		mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
		if (mRecorder.startRecording())
			setState(STATE_RECORDING);
	}

	private void startPlayback() {
		mRecorder.stop();
		mRecorder.startPlayback();
		setState(STATE_PLAYING);
	}

	private void stop() {
		mRecorder.stop();
		setState(STATE_RECORDED);
	}

	private void save() {
		mRecorder.stop();
		setState(STATE_IDLE);
		mRecorderListener.onRecordSaved(mRecorder.getAudioFile());
	}

	private void cancel() {
		mRecorder.delete();
		setState(STATE_IDLE);
	}

	public void onError(int error) {
		cancel();
		mTextView.setText("RecorderError");
	}

	public void onCompletion() {
		mTextView.setText("PlaybackComplete");
	}

}

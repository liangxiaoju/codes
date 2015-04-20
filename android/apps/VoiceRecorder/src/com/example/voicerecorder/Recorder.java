package com.example.voicerecorder;

import java.io.File;
import java.io.IOException;

import android.media.MediaRecorder;
import android.media.MediaPlayer;
import android.os.Environment;
import android.text.format.Time;

public class Recorder implements MediaPlayer.OnCompletionListener,
		MediaPlayer.OnErrorListener {

	public static final String PREFIX_RECORD = "recording-";
	public static final String PREFIX_CALL = "calling-";
	public static final String POSTFIX = ".3gpp";

	public static final int SDCARD_ACCESS_ERROR = 1;
	public static final int INTERNAL_ERROR = 2;

	private MediaRecorder mRecorder = null;
	private MediaPlayer mPlayer = null;
	private File mSampleDir = null;
	private File mSampleFile = null;
	private int mAudioSource = MediaRecorder.AudioSource.MIC;
	private OnRecorderListener mOnRecorderListener = null;

	public interface OnRecorderListener {
		public void onCompletion();

		public void onError(int error);
	}

	public void setAudioSource(int source) {
		mAudioSource = source;
	}

    public File getAudioFile() {
        return mSampleFile;
    }

	public boolean startRecording() {
		mSampleDir = Environment
				.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
		if (!mSampleDir.exists())
			mSampleDir.mkdir();

		Time time = new Time();
		time.setToNow();
		String timestr = time.format("%Y-%m-%d-%H-%M-%S");

		String filename;
		if (mAudioSource == MediaRecorder.AudioSource.VOICE_CALL) {
			filename = PREFIX_CALL + timestr + POSTFIX;
		} else {
			filename = PREFIX_RECORD + timestr + POSTFIX;
		}

		mSampleFile = new File(mSampleDir, filename);

		System.out.println(mSampleFile.getAbsolutePath());

		mRecorder = new MediaRecorder();
		mRecorder.setAudioSource(mAudioSource);
		mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
		mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
		mRecorder.setOutputFile(mSampleFile.getAbsolutePath());

		mRecorder.setOnErrorListener(new MediaRecorder.OnErrorListener() {

			@Override
			public void onError(MediaRecorder mr, int what, int extra) {
				// TODO Auto-generated method stub
				stop();
				setError(SDCARD_ACCESS_ERROR);
			}
		});

		try {
			mRecorder.prepare();
			mRecorder.start();
		} catch (IOException e) {
			System.out.println("error: Recorder prepare");
			setError(INTERNAL_ERROR);
			mRecorder.reset();
			mRecorder.release();
			mRecorder = null;
			return false;
		} catch (RuntimeException e) {
			System.out.println("error: Recorder start");
			setError(INTERNAL_ERROR);
			try {
				mRecorder.reset();
				mRecorder.release();
			} catch (IllegalStateException e1) {
				;
			}
			mRecorder = null;
			return false;
		}
		return true;
	}

	public boolean startPlayback() {
		mPlayer = new MediaPlayer();
		try {
			mPlayer.setDataSource(mSampleFile.getAbsolutePath());
			mPlayer.setOnCompletionListener(this);
			mPlayer.setOnErrorListener(this);
			mPlayer.prepare();
			mPlayer.start();
		} catch (IllegalArgumentException e) {
			setError(INTERNAL_ERROR);
			mPlayer = null;
			return false;
		} catch (IOException e) {
			setError(SDCARD_ACCESS_ERROR);
			mPlayer = null;
			return false;
		}
		return true;
	}

	public void stopRecording() {
		if (mRecorder != null) {
			mRecorder.stop();
			mRecorder.release();
			mRecorder = null;
		}
	}

	public void stopPlayback() {
		if (mPlayer != null) {
			mPlayer.stop();
			mPlayer.release();
			mPlayer = null;
		}
	}

	public void stop() {
		stopRecording();
		stopPlayback();
	}

	public void delete() {
		stop();
		if (mSampleFile != null)
			mSampleFile.delete();
	}

	private void setError(int error) {
		if (mOnRecorderListener != null)
			mOnRecorderListener.onError(error);
	}

	public void setRecorderListener(OnRecorderListener l) {
		mOnRecorderListener = l;
	}

	@Override
	public void onCompletion(MediaPlayer mp) {
		if (mOnRecorderListener != null)
			mOnRecorderListener.onCompletion();
	}

	@Override
	public boolean onError(MediaPlayer mp, int what, int extra) {
		stop();
		setError(SDCARD_ACCESS_ERROR);
		return true;
	}

}

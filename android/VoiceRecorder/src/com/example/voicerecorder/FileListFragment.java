package com.example.voicerecorder;

import java.io.File;
import java.io.FileFilter;

import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.Fragment;
import android.util.SparseBooleanArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class FileListFragment extends Fragment {


	private File mSampleDir;
	private File[] mFiles;

	private ListView mFileList;
	private ArrayAdapter<String> mAdapterNormalMode;
	private ArrayAdapter<String> mAdapterSelectMode;
	private ArrayAdapter<String> mFileListArrayAdapter;

	public static Fragment newInstance() {
		return new FileListFragment();
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View rootView = inflater.inflate(R.layout.fragment_filelist, container,
				false);

		mFileList = (ListView) rootView.findViewById(R.id.listview_filelist);

		mAdapterNormalMode = new ArrayAdapter<String>(getActivity(),
		// R.layout.list_item_filelist, R.id.item_textview);
		// android.R.layout.simple_list_item_multiple_choice);
				android.R.layout.simple_list_item_1);
		mAdapterSelectMode = new ArrayAdapter<String>(getActivity(),
		// R.layout.list_item_filelist, R.id.item_textview);
				android.R.layout.simple_list_item_multiple_choice);

		mFileListArrayAdapter = mAdapterNormalMode;
		mFileList.setAdapter(mFileListArrayAdapter);
		mFileList.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		mFileList.setLongClickable(true);
		mFileList.setOnItemLongClickListener(new OnItemLongClickListener() {

			@Override
			public boolean onItemLongClick(AdapterView<?> parent, View view,
					int position, long id) {
				toggleSelectMode(true);
				mFileList.performItemClick(view, position, id);
				return true;
			}

		});

		return rootView;
	}
	
	public boolean onBackPressed() {
		if (isSelectMode()) {
			toggleSelectMode(false);
			return true;
		} else
			return false;
	}
	
	public boolean onDelete() {
		SparseBooleanArray checkItems = mFileList.getCheckedItemPositions();
		int numCheckedItems = checkItems.size();
		System.out.println("onDelete num=" + numCheckedItems);
		for (int i = 0; i < numCheckedItems; i++) {
			if (!checkItems.valueAt(i))
				continue;
			int position = checkItems.keyAt(i);
			String name = mFileListArrayAdapter.getItem(position);
			remove(name);
			System.out.println("onDelete position=" + position);
		}
		toggleSelectMode(false);
		return true;
	}
	
	public boolean isSelectMode() {
		return (mFileListArrayAdapter == mAdapterSelectMode);
	}
	
	private void toggleSelectMode(boolean on) {
		ArrayAdapter<String> adapter = on ?  mAdapterSelectMode: mAdapterNormalMode;
		if (adapter != mFileListArrayAdapter) {
			mFileListArrayAdapter = adapter;
			rescan();
			mFileList.setAdapter(mFileListArrayAdapter);
		}
	}

	@Override
	public void onResume() {
		super.onResume();
		rescan();
	}

	@Override
	public void onPause() {
		super.onPause();
		toggleSelectMode(false);
	}

	public void rescan() {
		mSampleDir = Environment
				.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);

		if (mSampleDir.isDirectory()) {
			FileFilter ff = new FileFilter() {
				public boolean accept(File f) {
					String name = f.getName();
					return (name.startsWith(Recorder.PREFIX_RECORD) || name
							.startsWith(Recorder.PREFIX_CALL))
							&& name.endsWith(Recorder.POSTFIX);
				}
			};
			mFiles = mSampleDir.listFiles(ff);
		}

		clear();
		for (int i = 0; i < mFiles.length; i++) {
			add(mFiles[i]);
		}
	}

	public void add(File f) {
		mFileListArrayAdapter.insert(f.getName(), 0);
	}

	public void remove(String name) {
		mFileListArrayAdapter.remove(name);
		File file = new File(mSampleDir, name);
		file.delete();
	}

	public void clear() {
		mFileListArrayAdapter.clear();
	}

}

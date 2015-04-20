package com.example.voicerecorder;

import java.io.File;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

public class MainActivity extends FragmentActivity implements
		RecorderFragment.OnRecorderListener {

	private RecorderFragment mRecorderFragment;
	private FileListFragment mFileListFragment;

	SectionsPagerAdapter mSectionsPagerAdapter;

	ViewPager mViewPager;
	
	Menu mMenu;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		mSectionsPagerAdapter = new SectionsPagerAdapter(
				getSupportFragmentManager());
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);
		mViewPager.setOnPageChangeListener(new ViewPageChangeListener());
	}
	
	class ViewPageChangeListener implements OnPageChangeListener {
		
		@Override
		public void onPageScrollStateChanged(int arg0) {
			// TODO Auto-generated method stub

		}

		@Override
		public void onPageScrolled(int arg0, float arg1, int arg2) {
			// TODO Auto-generated method stub

		}

		@Override
		public void onPageSelected(int arg0) {
			// TODO Auto-generated method stub
			mMenu.findItem(R.id.action_delete).setVisible(arg0 == 1);
		}
	}

	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		if (mViewPager.getCurrentItem() == 1) {
			if (mFileListFragment.onBackPressed() == false)
				mViewPager.setCurrentItem(0, true);
		} else {
			super.onBackPressed();
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.voice_recorder, menu);
		mMenu = menu;
		return true;
	}
	
	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		// TODO Auto-generated method stub
		switch (item.getItemId()) {
		case R.id.action_delete:
			mFileListFragment.onDelete();
			break;
		case R.id.action_settings:
			break;
		}
		return super.onMenuItemSelected(featureId, item);
	}

	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			switch (position) {
			case 0:
				mRecorderFragment = (RecorderFragment) RecorderFragment
						.newInstance();
				return mRecorderFragment;
			case 1:
				mFileListFragment = (FileListFragment) FileListFragment
						.newInstance();
				return mFileListFragment;
			}
			return null;
		}

		@Override
		public int getCount() {
			return 2;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			switch (position) {
			case 0:
				return getString(R.string.title_recorder_fragment);
			case 1:
				return getString(R.string.title_filelist_fragment);
			}
			return null;
		}
	}

	public void onRecordSaved(File f) {
		mFileListFragment.add(f);
	}
}

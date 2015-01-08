package com.example.learnenglish;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.Uri.Builder;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class SentenceSectionFragment extends Fragment {

	/**
	 * The fragment argument representing the section number for this
	 * fragment.
	 */
	public static final String ARG_SECTION_NUMBER = "section_number";
	private TextView mSentenceTextView;
	private TextView mTv_1;
	private TextView mTv_2;
	private TextView mTv_3;
	private Bundle mBundle;
	private GrabData mResult = null;

	public SentenceSectionFragment() {
	}
	
	public static Fragment newInstance(int sectionNumber) {
		SentenceSectionFragment fragment = new SentenceSectionFragment();
		Bundle args = new Bundle();
		args.putInt(ARG_SECTION_NUMBER,  sectionNumber);
		fragment.setArguments(args);
		return fragment;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View rootView = inflater.inflate(R.layout.fragment_sentence_section,
				container, false);
		//mSentenceTextView = (TextView) rootView
		//		.findViewById(R.id.section_label);
		mTv_1 = (TextView) rootView.findViewById(R.id.sentence_1);
		mTv_2 = (TextView) rootView.findViewById(R.id.sentence_2);
		mTv_3 = (TextView) rootView.findViewById(R.id.sentence_3);
		
		return rootView;
	}

	@Override
	public void onResume() {
		// TODO Auto-generated method stub

		int section = getArguments().getInt(ARG_SECTION_NUMBER);
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.DATE, -section);
		
		if (mResult == null) {
			DataGrabTask grab = new DataGrabTask(new DataGrabTask.Callback() {

				@Override
				public void done(GrabData result) {
					// TODO Auto-generated method stub
					//mSentenceTextView.setText(result.toString());
					mResult = result;
					mTv_1.setText(result.document.content);
					mTv_2.setText(result.document.note);
					mTv_3.setText(result.document.translation);
				}
			});

			ConnectivityManager connMgr = (ConnectivityManager)
					getActivity().getSystemService(Context.CONNECTIVITY_SERVICE);
			NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
			if (networkInfo != null && networkInfo.isConnected()) {
				grab.execute(UrlUtil.generateUrl(cal.getTime()));
			} else {
				mSentenceTextView.setText(getString(R.string.network_not_connected));
			}
			
		} else {
			//mSentenceTextView.setText(mResult.toString());
			mTv_1.setText(mResult.document.content);
			mTv_2.setText(mResult.document.note);
			mTv_3.setText(mResult.document.translation);
		}
		
		super.onResume();
	}

	@Override
	public void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
	}


	
	public static class UrlUtil {
		
		public static String generateUrl(Date d) {

			SimpleDateFormat fmt = new SimpleDateFormat("yyyy-MM-dd", Locale.getDefault());

			String date = fmt.format(d);
			//http://open.iciba.com/dsapi/?date=2014-1-22&file=xml&type=next
			//{@code <scheme>://<authority><absolute path>?<query>#<fragment>}
			Builder builder = new Uri.Builder();

			builder.scheme("http")
				.authority("open.iciba.com")
				.appendPath("dsapi")
				.appendQueryParameter("file", "xml")
				.appendQueryParameter("date", date);
			
			//Uri uri = builder.build();

			System.out.println(builder.toString());
			
			return builder.toString();
		}
	}
}


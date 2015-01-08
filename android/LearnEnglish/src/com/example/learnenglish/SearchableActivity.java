package com.example.learnenglish;

import android.net.ConnectivityManager;
import android.os.Bundle;
import android.app.Activity;
import android.app.SearchManager;
import android.content.Context;
import android.content.Intent;
import android.text.Html;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.SearchView;
import android.widget.TextView;

public class SearchableActivity extends Activity {
	
	SearchView mSearchView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_searchable);
		
		Intent intent = getIntent();
		handleIntent(intent);
	}
	
	@Override
	protected void onNewIntent(Intent intent) {
		setIntent(intent);
		handleIntent(intent);
	}
	
	private void handleIntent(Intent intent) {
		if (Intent.ACTION_SEARCH.equals(intent.getAction())) {
			String query = intent.getStringExtra(SearchManager.QUERY);
			doMySearch(query);
		}
	}

	public void doMySearch(String query) {
		final TextView textView = (TextView) findViewById(R.id.search_result_tv);
		textView.setText("Key: " + query);
		
		DataGrabTask grab = new DataGrabTask(new DataGrabTask.Callback() {

			@Override
			public void done(GrabData result) {
				// TODO Auto-generated method stub
				textView.setText(result.toSpanned());
			}
		});
		String url = String.format(
				"http://dict-co.iciba.com/api/dictionary.php?w=%s&key=F5659FA74354A3BDA64C48487254DC5F", query);
		grab.execute(url);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.searchable, menu);
		
		SearchManager searchManager = (SearchManager) getSystemService(Context.SEARCH_SERVICE);
		mSearchView = (SearchView) menu.findItem(R.id.action_search).getActionView();
		mSearchView.setSearchableInfo(searchManager.getSearchableInfo(getComponentName()));
		mSearchView.setIconifiedByDefault(false);
		
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
		switch (item.getItemId()) {
		case R.id.action_search:
			onSearchRequested();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

}
		
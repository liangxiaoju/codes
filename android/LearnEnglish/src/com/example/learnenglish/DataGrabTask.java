package com.example.learnenglish;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.HttpURLConnection;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.R.integer;
import android.net.Uri;
import android.net.Uri.Builder;
import android.net.http.*;
import android.os.AsyncTask;
import android.util.Log;
import android.util.Xml;

import com.example.learnenglish.GrabData.*;

public class DataGrabTask extends AsyncTask<String, Integer, GrabData>{

	public static final String TAG = "DataGrab";
	
	Callback mCallback;

	public interface Callback {
		void done(GrabData result);
	}
	
	@Override
	protected GrabData doInBackground(String... params) {
		// TODO Auto-generated method stub
		try {
			InputStream in = getSentence(params[0]);
			XmlParser parser = new XmlParser();
			return parser.parse(in);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return new GrabData();
		} catch (XmlPullParserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return new GrabData();
		}
	}

	@Override
	protected void onProgressUpdate(Integer... values) {
		// TODO Auto-generated method stub
		super.onProgressUpdate(values);
	}

	@Override
	protected void onPostExecute(GrabData result) {
		Log.d(TAG, result.toString());
		mCallback.done(result);
	}

	public DataGrabTask(Callback call) {
		mCallback = call;
	}

	public InputStream getSentence(String u) throws IOException {

		URL url;
		HttpURLConnection conn = null;
		InputStream in = null;
		
		try {
			url = new URL(u);
			conn = (HttpURLConnection) url.openConnection();
			conn.setReadTimeout(10000);
			conn.setConnectTimeout(15000);
			conn.setRequestMethod("GET");
			conn.setDoInput(true);
			conn.connect();
			int response = conn.getResponseCode();
			Log.d(TAG, "The response is: " + response);
			in = new BufferedInputStream(conn.getInputStream());
		} finally {
		//	if (conn != null)
		//		conn.disconnect();
		}
		return in;
	}
	
	private String readStream(InputStream in) {
		BufferedReader reader = new BufferedReader(new InputStreamReader(in));
		StringBuffer text = new StringBuffer("");
		String line;
		try {
			while ((line = reader.readLine()) != null) {
				text.append(line);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return text.toString();
	}
	
	public class XmlParser {
		public GrabData parse(InputStream in) throws XmlPullParserException, IOException {
			try {
				XmlPullParser parser = Xml.newPullParser();
				parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
				parser.setInput(in, null);
				parser.nextTag();
				return readGrabData(parser);
			} finally {
				in.close();
			}
		}
		
		private GrabData readGrabData(XmlPullParser parser) throws XmlPullParserException, IOException {
			parser.require(XmlPullParser.START_TAG, null, null);
			String tag = parser.getName();
			if (tag.equals("Document")) {
				return readDocument(parser);
			} else if (tag.equals("dict")) {
				return readDict(parser);
			} else {
				return null;
			}
		}
		
		private String readText(XmlPullParser parser) throws XmlPullParserException, IOException {
			String result = "";
			if (parser.next() == XmlPullParser.TEXT) {
				result = parser.getText();
				parser.nextTag();
			}
			return result;
		}
		
		private int readNumByName(XmlPullParser parser, String name) throws XmlPullParserException, IOException {
			int num;
			parser.require(XmlPullParser.START_TAG, null, name);
			try {
				num = Integer.parseInt(readText(parser));
			} catch (NumberFormatException e) {
				num = 0;
			}
			parser.require(XmlPullParser.END_TAG, null, name);
			return num;
		}
		
		private String readTextByName(XmlPullParser parser, String name) throws XmlPullParserException, IOException {
			parser.require(XmlPullParser.START_TAG, null, name);
			String text = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, name);
			return text;
		}
		
		private void skip(XmlPullParser parser) throws XmlPullParserException, IOException {
			parser.require(XmlPullParser.START_TAG, null, null);
			int depth = 1;
			while (depth != 0) {
				switch (parser.next()) {
				case XmlPullParser.END_TAG:
					depth--;
					break;
				case XmlPullParser.START_TAG:
					depth++;
					break;
				}
			}
		}
		
		private GrabData readDocument(XmlPullParser parser) throws XmlPullParserException, IOException {
			GrabData doc = new GrabData();
			doc.isDocument = true;
			doc.isDict = false;
			
			parser.require(XmlPullParser.START_TAG, null, "Document");

			while (parser.next() != XmlPullParser.END_TAG) {
				if (parser.getEventType() != XmlPullParser.START_TAG) {
					continue;
				}
				String name = parser.getName();
				if (name.equals("sid")) {
					doc.document.sid = readNumByName(parser, name);
				} else if (name.equals("tts")) {
					doc.document.tts = readTextByName(parser, name);
				} else if (name.equals("content")) {
					doc.document.content = readTextByName(parser, name);
				} else if (name.equals("note")) {
					doc.document.note = readTextByName(parser, name);
				} else if (name.equals("translation")) {
					doc.document.translation = readTextByName(parser, name);
				} else if (name.equals("picture")) {
					doc.document.picture = readTextByName(parser, name);
				} else if (name.equals("picture2")) {
					doc.document.picture2 = readTextByName(parser, name);
				} else if (name.equals("dateline")) {
					doc.document.dateline = readTextByName(parser, name);
				} else {
					skip(parser);
				}
			}
			
			return doc;
		}
		
		private Pron readPron(XmlPullParser parser) throws XmlPullParserException, IOException {
			Pron pron = new Pron();
			parser.require(XmlPullParser.START_TAG, null, "ps");
			pron.ps = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, "ps");
			parser.nextTag();
			parser.require(XmlPullParser.START_TAG, null, "pron");
			pron.uri = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, "pron");
			return pron;
		}

		private Acceptation readAcceptation(XmlPullParser parser) throws XmlPullParserException, IOException {
			Acceptation acceptation = new Acceptation();
			parser.require(XmlPullParser.START_TAG, null, "pos");
			acceptation.pos = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, "pos");
			parser.nextTag();
			parser.require(XmlPullParser.START_TAG, null, "acceptation");
			acceptation.acceptation = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, "acceptation");
			return acceptation;
		}
		
		private Sent readSent(XmlPullParser parser) throws XmlPullParserException, IOException {
			Sent sent = new Sent();
			parser.require(XmlPullParser.START_TAG, null, "sent");
			parser.nextTag();
			parser.require(XmlPullParser.START_TAG, null, "orig");
			sent.orig = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, "orig");
			parser.nextTag();
			parser.require(XmlPullParser.START_TAG, null, "trans");
			sent.trans = readText(parser);
			parser.require(XmlPullParser.END_TAG, null, "trans");
			parser.nextTag();
			parser.require(XmlPullParser.END_TAG, null, "sent");
			return sent;
		}		
		private GrabData readDict(XmlPullParser parser) throws XmlPullParserException, IOException {
			GrabData dict = new GrabData();
			dict.isDocument = false;
			dict.isDict = true;
			
			parser.require(XmlPullParser.START_TAG, null, "dict");

			while (parser.next() != XmlPullParser.END_TAG) {
				if (parser.getEventType() != XmlPullParser.START_TAG) {
					continue;
				}
				String name = parser.getName();
				if (name.equals("key")) {
					dict.dict.key = readTextByName(parser, name);
				} else if (name.equals("ps")) {
					dict.dict.prons.add(readPron(parser));
				} else if (name.equals("pos")) {
					dict.dict.acceptations.add(readAcceptation(parser));
				} else if (name.equals("sent")) {
					dict.dict.sents.add(readSent(parser));
				} else {
					skip(parser);
				}
			}
			
			return dict;
		}
	}
	
}

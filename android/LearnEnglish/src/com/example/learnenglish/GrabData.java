package com.example.learnenglish;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.net.Uri;
import android.os.Bundle;
import android.text.Html;
import android.text.Spanned;

class Document {
	int sid;
	String tts;
	String content;
	String note;
	int love;
	String translation;
	String picture;
	String picture2;
	String caption;
	String dateline;
	int s_pv;
	int sp_pv;
	int[] tagid;
	String tag;
	String fenxiang_img;
}

class Pron {
	String uri;
	String ps;
}

class Acceptation {
	String pos;
	String acceptation;
}

class Sent {
	String orig;
	String trans;
}

class Dict {

	String key;
	List<Pron> prons;
	List<Acceptation> acceptations;
	List<Sent> sents;
}

public class GrabData {

	Document document;
	Dict dict;
	String data;
	boolean isDocument;
	boolean isDict;

	public GrabData() {
		document = new Document();
		dict = new Dict();
		dict.prons = new ArrayList<Pron>();
		dict.acceptations = new ArrayList<Acceptation>();
		dict.sents = new ArrayList<Sent>();
	}

	public String toString() {
		String s = "";
		if (isDocument) {
			s = document.content + "\n\n" + document.note + "\n\n"
					+ document.translation + "\n\n" + document.dateline;
		}
		if (isDict) {
			s = dict.key + "\n";
			for (Pron pron : dict.prons) {
				s += "[" + pron.ps + "]" + "\t";
			}
			s += "\n";
			for (Acceptation accept : dict.acceptations) {
				s += accept.pos + accept.acceptation;
			}
			s += "\n";
			for (Sent sent : dict.sents) {
				s += sent.orig + sent.trans + "\n";
			}
		}
		return s;
	}
	
	public Spanned toSpanned() {
		String s = "";
		if (isDocument) {
			s = document.content + "\n\n" + document.note + "\n\n"
					+ document.translation + "\n\n" + document.dateline;
		}
		if (isDict) {
			s = "<h4>" + dict.key + "</h4>";
			
			s += "<hr />";
			
			s += "<p>";
			for (Pron pron : dict.prons) {
				s += "[" + pron.ps + "]" + "    ";
			}
			s += "</p>";
			
			s += "<hr />";
			
			s += "<p>";
			for (Acceptation accept : dict.acceptations) {
				s += "<b>" + accept.pos + "</b>" + "<br />" + accept.acceptation + "<br />";
			}
			s += "</p>";
			s += "<br />";
			
			s += "<hr />";
			s += "<hr />";
			
			s += "<p>";
			for (Sent sent : dict.sents) {
				s += "<i><b>" + sent.orig + "</b></i>"
						+ "<br />" + sent.trans + "<br /><br />";
			}
			s += "</p>";
		}
		System.out.println(s);
		return Html.fromHtml("<html><body>" + s + "</body></html>");
	}
}

package com.example.voicerecorder;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Command {

	String mCmd;
	String mResult;

	public Command(String cmd) {
		mCmd = cmd;
	}

	public boolean exec() {
		try {
			// Executes the command.
			Process process = Runtime.getRuntime().exec(mCmd);

			// Reads stdout.
			// NOTE: You can write to stdin of the command using
			// process.getOutputStream().
			BufferedReader reader = new BufferedReader(new InputStreamReader(
					process.getInputStream()));
			int read;
			char[] buffer = new char[4096];
			StringBuffer output = new StringBuffer();
			while ((read = reader.read(buffer)) > 0) {
				output.append(buffer, 0, read);
			}
			reader.close();

			// Waits for the command to finish.
			process.waitFor();

			mResult = output.toString();
		} catch (IOException e) {
			System.out.println("IOException");
			return false;
		} catch (InterruptedException e) {
			System.out.println("InterruptedException");
			return false;
		}

		return true;
	}
	
	public String result() {
		return mResult;
	}
}

package com.ftptest;

import java.io.PrintStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.lang.reflect.Field;

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPClientConfig;
import org.apache.commons.net.ftp.FTPConnectionClosedException;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;

public class FTPTestMain {
	
	public static void main (String[] args) {
		System.out.println ("main");

		FTPClient fc = new FTPClient();


		while (true) {

			try {
				fc.connect ("127.0.0.1", 50000);

				try {
					Thread.sleep (300);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}

			} catch (Exception e) {
				e.printStackTrace();

			} finally {
//				try {
//					fc.disconnect();
//				} catch (Exception e) {
//				}
			}
		}

	}
}

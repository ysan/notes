package otram;

import java.io.*;
import java.util.*;
import java.text.*;

public class Otram {

	public static void main(String[] args) {
		System.out.println("main");
		
		Date date = null;
		try {
			String strDate = "2017/05/01 01:00:00";
			
			SimpleDateFormat sdFormat = new SimpleDateFormat ("yyyy/MM/dd hh:mm:ss");
			date = sdFormat.parse(strDate);
			
		} catch (ParseException e) {
			e.printStackTrace();
		}
		
		System.out.println("Date " + date);
		System.out.println("Date " + date.getTime());
		long off = 30*24*60*60*1000L;
		System.out.println(off);
		long h = date.getTime()+(30*24*60*60*1000L);
		long l = date.getTime()-(30*24*60*60*1000L);
		System.out.println (l + " - " + h);
		
		File dir = new File ("/home/yoshi/prog");
	        
		File[] list = dir.listFiles();
		if (list == null) {
			System.exit (1);
		}
		
		for (int i = 0; i < list.length; ++ i) {
			File f = list [i];
			if (f.isDirectory()) {
				continue;
			}
			
			long lm = f.lastModified();
			if ((lm >= l) && (lm <= h)) {
				System.out.println (f.getAbsolutePath());
//				System.out.println (f.lastModified());
//				Date d = new Date (f.lastModified());
//				System.out.println (d);
			}
		}
		
	}

}

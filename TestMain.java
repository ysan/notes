package ppp;

import java.io.*;
import java.util.regex.*;

public class TestMain {

	public static void main (String args[]) {
		
		System.out.println ("main start");
		
		TestMain t =  new TestMain();
		t.fileRead ("/home/yoshi/prog/ws_ppp/ppp/src/ppp/test.txt");
		
		
		System.out.println ("main end");
	}
	
	private void fileRead (String filePath) {
		FileReader fr = null;
		BufferedReader br = null;
		
		try {
			fr = new FileReader (filePath);
			br = new BufferedReader (fr);
	 
			String pattern = "^ *# *RESULT_PASS *=";
			Pattern p = Pattern.compile(pattern);
			
			String line = "";
			String r = "";
			while ((line = br.readLine()) != null) {
				System.out.println (line);
				
				Matcher m = p.matcher(line);
				if (m.find()) {
					System.out.println (line);
					System.out.println ("match: [" + m.group() + "]");
					
					String msub = line.substring(m.end());
					System.out.println ("msub: [" + msub + "]");
					
					String rep1 = msub.replaceAll(" ", "[sp]");
					String rep2 = rep1.replaceAll("\\\\", "\\\\\\\\");
					r = rep2;
					
					break;
				}
				
			}
			
			if (!r.isEmpty()) {
				System.out.println ("r: [" + r + "]");

			} else {
				System.out.println ("not found...");
			}
			
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				br.close();
				fr.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}

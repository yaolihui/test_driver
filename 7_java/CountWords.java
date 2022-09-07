import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class CountWords {
	private static String DIR="/home/yaolihui/d/WSL2-Linux-Kernel/Documentation";
	private static boolean needPool = true;

	private static Hashtable<String, Integer> table = new Hashtable<String, Integer>();
	private static ThreadPoolExecutor pool = new ThreadPoolExecutor(10, 100, 100, TimeUnit.MILLISECONDS, 
													new LinkedBlockingDeque<Runnable>(), 
													Executors.defaultThreadFactory(), 
													new ThreadPoolExecutor.CallerRunsPolicy()){
														protected void terminated() {
															printSortedResult();
															System.out.println("total time:" + (System.currentTimeMillis() - s) + "ms");
														}
													};

	private static boolean isWord(String word) {
		char[] chars = word.toCharArray();
		for (char c : chars) {
			if (c < 'a' || c > 'z') {
				//System.out.println(c);
				return false;				
			}
		}
		return true;
	}
	
	private static void readFile(File f) {
		//System.out.println("read file:" + f.getPath());
		String str = null;
		BufferedReader br = null;		
		try {
			br =new BufferedReader(new InputStreamReader(new FileInputStream(f)));
			while((str = br.readLine()) != null) {
				String[] wds = str.split(" |\n|\r|\t|@|:|=|/|\\^|\\.|\\(|\\)|\\<|\\>|-|_|,|#|\"|\\[|\\]|\\+|\\|\\!|“|”|\\{|\\}|`|\\!|\\|\\?|;|\'|\\\\|[0-9]");
				if (null != wds) {
					for (String wd: wds) {
						String w = wd.trim().toLowerCase();
						//System.out.println(w);
						if (w.length() > 2 && w.length() < 16 && isWord(w)) {
							Integer count = table.get(w);
							//System.out.println("count:" + count);
							if (null == count) {
								table.put(w, new Integer(1));
							} else {
								table.put(w, ++count);
							}
						}
					}
				}
			}
		} catch (Exception e) {
			System.out.println("e:" + e);
		} finally {
			if (null != br) {
				try {
					br.close();
				} catch (Exception e1) {
					System.out.println("e:" + e1);
				} finally {
					br = null;
				}
			}
		}
	}

	private static void printSortedResult() {
		List<String> list = new ArrayList<String>(table.keySet());
		Collections.sort(list, (arg0, arg1)->{
			return table.get(arg1) - table.get(arg0);
		});
		for(String k: list) {
			System.out.println(k + "\t\t\t" + table.get(k));
		}
		System.out.println("==================================\ntotal words:" + list.size());
	}

	private static void traverse(File file) {
		//System.out.println("file=" + file);
		if (file.isDirectory()) {
			File[] files = file.listFiles(pathname -> {
				if (pathname.isFile()){
					String name = pathname.getName();
					int loc = name.indexOf(".");
					if (loc > 0) {
						String ext = name.substring(loc);
						return ".rst".equals(ext) || ".txt".equals(ext);
					}
				}
				return true;
			});
			if (null != files){
				//System.out.println("files.length="+ files.length);
				for(File dir: files) {
					traverse(dir);
				}
			}
		} else {
			if (needPool) {
				pool.execute(()->{
					//System.out.println("ActiveCount:" + pool.getActiveCount());
					readFile(file);
				});
			} else {
				readFile(file);
			}
		}
	}
	
	
	private static long s = System.currentTimeMillis();
	public static void main(String[] args) {
		//System.out.println("hello world!");
		traverse(new File(DIR));
		
		if (needPool) {
			pool.shutdown();
		} else {
			printSortedResult();
			System.out.println("total time:" + (System.currentTimeMillis() - s) + "ms");
		}
	}
}

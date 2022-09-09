import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class CountWords {
	private static final boolean THREAD_POOL = true;

	private static Hashtable<String, Integer> table = new Hashtable<String, Integer>();
	private static ThreadPoolExecutor pool =
		new ThreadPoolExecutor(100/*corePoolSize*/, 1000/*maximumPoolSize*/, 100/*keepAliveTime*/, TimeUnit.MILLISECONDS/*unit*/, 
								new LinkedBlockingDeque<Runnable>(), 
								Executors.defaultThreadFactory(), 
								new ThreadPoolExecutor.CallerRunsPolicy()){
									protected void terminated() {
										printSortedResult();
										System.out.println("total time:" + (System.currentTimeMillis() - s) + "ms");
									}
								};

	private static boolean isFakeWord(String word) {
		char[] chars = word.toCharArray();
		int length = word.length();
		
		if ('x' == chars[0]) {
			return true;
		}

		if (length >= 3) { 
			for (int i = 0; i <= length - 3; i++) {
				if ((chars[i] == chars[i+1] && chars[i] == chars[i+2]) /*aaa*/
				|| (i <= length - 4 && chars[i] == chars[i+1] && chars[i+2] == chars[i+3]) /*abab*/) {
					return true;
				}
			}
		}
		return false;
	}

	private static boolean isWord(String word) {
		for (char c : word.toCharArray()) {
			if (c < 'A' || c > 'z') {
				//System.out.println(c);
				return false;
			}
		}
		return true;
	}

	private static boolean isSuitedWord(String word) {
		int length = word.length();
		return length > 3 && length < 16;
	}

	private static final String REGEX = " |[0-9]|\r|\t|@|:|=|-|_|,|#|;|\'|`|\"|“|”|/|\\^|\\.|\\*|\\(|\\)|\\<|\\>|\\[|\\]|\\+|\\|\\!|\\{|\\}|\\!|\\|\\?|\\\\";
	private static void readFile(File f) {
		//System.out.println("read file:" + f.getPath());
		String str = null;
		BufferedReader br = null;
		try {
			br = new BufferedReader(new InputStreamReader(new FileInputStream(f)));
			while((str = br.readLine()) != null) {
				String[] wds = str.split(REGEX);
				if (null != wds) {
					for (String wd: wds) {
						String w = wd.trim().toLowerCase();
						//System.out.println(w);
						if (isSuitedWord(w) && isWord(w) && !isFakeWord(w)) {
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
			e.printStackTrace();
		} finally {
			closeReader(br);
		}
	}

	private static void closeReader(Reader r) {
		if (null != r) {
			try {
				r.close();
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				r = null;
			}
		}
	}

	private static void readFile2(File f) {
		Runnable r = () -> readFile(f);
		if (THREAD_POOL) {
			pool.execute(r);
		} else {
			r.run();
		}
	}

	private static void printSortedResult() {
		LinkedList<String> list = new LinkedList<String>(table.keySet());

		Collections.sort(list, (arg0, arg1)->{
			return arg0.compareTo(arg1);
		});

		Collections.sort(list, (arg0, arg1)->{
			return (table.get(arg1) - table.get(arg0));
		});

		for(String k: list) {
			System.out.println(k + "\t\t\t" + table.get(k));
		}

		System.out.println("--------------------------");
		System.out.println("total words:" + list.size());
	}

	private static boolean traverse(File file) {
		//System.out.println("file=" + file);
		if (file.isDirectory()) {
			file.listFiles(f -> traverse(f));
		} else {
			readFile2(file);
		}
		return true || false;
	}

	private static final String DIR="/home/yaolihui/d/WSL2-Linux-Kernel/Documentation";
	private static long s = System.currentTimeMillis();
	public static void main(String[] args) {
		//System.out.println("hello world!");
		traverse(new File(DIR));

		if (THREAD_POOL) {
			pool.shutdown();
		} else {
			printSortedResult();
			System.out.println("total time:" + (System.currentTimeMillis() - s) + "ms");
		}
	}
}

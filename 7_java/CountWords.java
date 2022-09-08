import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class CountWords {
    private static final String DIR="/home/yaolihui/d/WSL2-Linux-Kernel/Documentation";
    private static final boolean NEED_POOL = true;

    private static Hashtable<String, Integer> table = new Hashtable<String, Integer>();
    private static ThreadPoolExecutor pool = new ThreadPoolExecutor(100/*corePoolSize*/, 1000/*maximumPoolSize*/, 100/*keepAliveTime*/, TimeUnit.MILLISECONDS/*unit*/, 
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
            if (c < 'A' || c > 'z') {
                //System.out.println(c);
                return false;
            }
        }
        return true;
    }

    private static boolean isSuitedWord(String word) {
        return word.length() > 2 && word.length() < 16;
    }

    private static final String REGEX = " |[0-9]|\r|\t|@|:|=|-|_|,|#|;|\'|`|\"|“|”|/|\\^|\\.|\\*|\\(|\\)|\\<|\\>|\\[|\\]|\\+|\\|\\!|\\{|\\}|\\!|\\|\\?|\\\\";
    private static void readFile(File f) {
        //System.out.println("read file:" + f.getPath());
        String str = null;
        BufferedReader br = null;        
        try {
            br =new BufferedReader(new InputStreamReader(new FileInputStream(f)));
            while((str = br.readLine()) != null) {
                
                String[] wds = str.split(REGEX);
                if (null != wds) {
                    for (String wd: wds) {
                        String w = wd.trim().toLowerCase();
                        //System.out.println(w);
                        if (isSuitedWord(w) && isWord(w)) {
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
            if (NEED_POOL) {
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
        
        if (NEED_POOL) {
            pool.shutdown();
        } else {
            printSortedResult();
            System.out.println("total time:" + (System.currentTimeMillis() - s) + "ms");
        }
    }
}

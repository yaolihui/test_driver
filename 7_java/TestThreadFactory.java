import java.util.concurrent.*;

public class  TestThreadFactory {
    
    public static void main(String[] args) {
        ThreadFactory f = Executors.defaultThreadFactory();
        
        f.newThread(()->System.out.println("Hello World !!!")).start();
     
        new Thread(()->System.out.println("Hello World 222")).start();
        
    }
}
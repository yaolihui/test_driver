import java.util.*;
import java.util.function.*;


public class TestConsumer {


	public static void main(String[] args) {
		System.out.println("Hollo World!");
		
		ArrayList<String> list = new ArrayList<String>();
		
		Consumer<ArrayList<String>> c1 = new Consumer<ArrayList<String>>(){ 
			public void accept(ArrayList<String> t){
				t.add("111");
			}
		};		
		Consumer<ArrayList<String>> c2 = t -> t.add("111");		
		Consumer<ArrayList<String>> c3 = t -> System.out.println(t.size());
		
		System.out.println("------------------");
		
		c1.andThen(c2).andThen(c3).accept(list);
	}
	
}
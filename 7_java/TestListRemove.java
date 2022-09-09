
import java.util.*;

public class TestListRemove {
	
	public static void main(String[] args) {
		System.out.println("Hello World");
		ArrayList<Integer> list = new ArrayList<Integer>();
		for (int i =0; i< 10; i++) {
			list.add(new Integer(i));
		}
		
		for (int i =0; i< 10; i++) {
			if (list.contains(8)) {
				list.remove(8);
				// i--;
			}
		}
		
		for(Integer i: list) {
			System.out.println(i);
		}
		
	}
}

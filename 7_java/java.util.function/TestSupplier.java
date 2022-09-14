import java.util.*;
import java.util.function.*;

public class TestSupplier {


	public static void main(String[] args) {
		System.out.println("Hollo World!");

		Supplier<String> s1 = () -> "hello";
		System.out.println("Supplier:" + s1.get());

		Supplier<Long> s2 = () -> 2L;
		System.out.println("Supplier:" + s2.get());
	}
	
}
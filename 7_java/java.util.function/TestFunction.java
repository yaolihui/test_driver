import java.util.*;
import java.util.function.*;

public class TestFunction {
	
	
	public static void main(String[] args) {
		System.out.println("Hollo World!");
		
		Function<String, String> f1 = t ->  "(f1:" + t + ")";
		System.out.println(f1.apply("1"));
		
		Function<String, String> f2 = t -> "[f2:" + t + "]";
		System.out.println(f2.compose(f1).apply("2"));
		
		
		Function<String, String> f3 = t -> "<f3:" + t + ">";
		System.out.println(f3.compose(f2).compose(f1).apply("3"));
		
		Function<String, String> f4 = t -> "{f4:" + t + "}";
		System.out.println(f4.compose(f3).compose(f2).compose(f1).apply("4"));
		System.out.println(f4.compose(f3).compose(f2).andThen(f1).apply("4"));
		System.out.println(f4.compose(f3).andThen(f2).compose(f1).apply("4"));
		System.out.println(f4.andThen(f3).compose(f2).compose(f1).apply("4"));

		System.out.println(f4.andThen(f3).andThen(f2).compose(f1).apply("4"));
		System.out.println(f4.compose(f3).andThen(f2).andThen(f1).apply("4"));
		System.out.println(f4.andThen(f3).andThen(f2).compose(f1).apply("4"));
		System.out.println(f4.compose(f1).andThen(f3).andThen(f2).apply("4"));
	}
	
}
/*
(f1:1)
[f2:(f1:2)]
<f3:[f2:(f1:3)]>
{f4:<f3:[f2:(f1:4)]>}
(f1:{f4:<f3:[f2:4]>})
[f2:{f4:<f3:(f1:4)>}]
<f3:{f4:[f2:(f1:4)]}>
[f2:<f3:{f4:(f1:4)}>]
(f1:[f2:{f4:<f3:4>}])
[f2:<f3:{f4:(f1:4)}>]
[f2:<f3:{f4:(f1:4)}>]
*/
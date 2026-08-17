import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Method;
import java.lang.reflect.Parameter;

@Retention(RetentionPolicy.RUNTIME)
@interface MyAnnotation {
    String value() default "";
}
 
public class TestAnnotation {
    @MyAnnotation("Hello")
    public static void myMethod() {}    
    
    @MyAnnotation("Hello2")
    public static void myMethod2(@MyAnnotation(value = "param1")String param1, @MyAnnotation String param2) {}
    
    public static void myMethod3() {}
 
    public static void main(String[] args) {
        Method[] methods = TestAnnotation.class.getDeclaredMethods();

        for (Method method : methods) {
            if (method.isAnnotationPresent(MyAnnotation.class)) {
                System.out.println("isAnnotationPresent:" + method.getName());

                MyAnnotation annotation = method.getAnnotation(MyAnnotation.class);
                System.out.println("注解值：" + annotation.value());

                Parameter[] parameters = method.getParameters();
                for (Parameter parameter : parameters) {
                    if (parameter.isAnnotationPresent(MyAnnotation.class)) {
                        MyAnnotation annotationP = parameter.getAnnotation(MyAnnotation.class);
                        System.out.println("Parameter: " + parameter.getName() + ", is annotated with: " + annotationP.value());
                    }
                }
            } else {
                System.out.println("isAnnotationPresent NOT:" + method.getName());
            }
        }
    }
}


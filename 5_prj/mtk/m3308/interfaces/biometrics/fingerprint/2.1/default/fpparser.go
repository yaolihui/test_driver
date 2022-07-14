package fpparser

import (
        "android/soong/android"
        "android/soong/cc"
		"fmt"
)

func init() {
    // resister a module "xxxparser_defaults"
    android.RegisterModuleType("cc_fpparser_binary", fpdroidDefaultsFactory)
	fmt.Println("Hello, World3!")
}

func fpdroidDefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, fpdroidDefaults)
	fmt.Println("Hello, World0!")
    return module
}

func fpdroidDefaults(ctx android.LoadHookContext) {
   type props struct {
        Cflags []string
    }
    p := &props{}
    p.Cflags = globalDefaults(ctx)
    ctx.AppendProperties(p)
	fmt.Println("Hello, World1!")
}

func globalDefaults(ctx android.BaseContext) ([]string) {
    var cppflags []string
    if ctx.AConfig().Getenv("TARGET_PRODUCT") == "vnd_Romanee" {
          cppflags = append(cppflags,"-DROMANEE")
			fmt.Println("rafe test!")
    }
	fmt.Println("Hello, World2!")
    return cppflags
}

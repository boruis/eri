#ifndef ERI_ANDROID_HELPER_H
#define ERI_ANDROID_HELPER_H

#include <assert.h>

#include <jni.h>
#include <android_native_app_glue.h>
extern android_app* g_android_app;
extern const char* g_class_name;

struct JavaCaller
{
    JavaCaller()
    {
        jint res = g_android_app->activity->vm->AttachCurrentThread(&env, NULL);
        assert(res == 0);

        jclass activity_class = env->GetObjectClass(g_android_app->activity->clazz);
        assert(activity_class);
        
        jmethodID getClassLoader = env->GetMethodID(activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
        assert(getClassLoader);

        class_loader_ins = env->CallObjectMethod(g_android_app->activity->clazz, getClassLoader);
        assert(class_loader_ins);

        jclass class_loader_class = env->GetObjectClass(class_loader_ins);
        assert(class_loader_class);

        loadClass = env->GetMethodID(class_loader_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        assert(loadClass);

        attached = true;
    }

    ~JavaCaller()
    {
        assert(!attached);
    }

    void Set(const char* class_name, const char* func_name, const char* func_sig)
    {
        assert(attached);

        jstring jstr_class_Name = env->NewStringUTF(class_name);
        user_class = (jclass)env->CallObjectMethod(class_loader_ins, loadClass, jstr_class_Name);
        env->DeleteLocalRef(jstr_class_Name);
        assert(user_class);

        user_func = env->GetStaticMethodID(user_class, func_name, func_sig);
        assert(user_func);
    }

    void End()
    {
        assert(attached);

        g_android_app->activity->vm->DetachCurrentThread();

        attached = false;
    }

    JNIEnv* env;
    jobject class_loader_ins;
    jmethodID loadClass;
    jclass user_class;
    jmethodID user_func;

    bool attached;
};

#endif

##实现main-plugins-thread的三级命名
##每个插件都是一个线程，并且会拉起自己的子线程
##每个插件会动态记录下自己拉起了多少线程

##编译记录：1.name.cpp 中引用 name.h 时，若用 g++ -o test main.cpp name.cpp -lpthread编译，就会报错count重复定义，
            不加name.cpp的话又显示threadsetname函数无定义。
           2. name.cpp 中不引用 name.h ：g++ -o test main.cpp name.cpp -lpthread可以编译，不加name.cpp显示未定义
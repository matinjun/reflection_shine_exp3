# 实现光照与投影（基于**实验3.4**与**实验3.2**）
* 首先在[顶点着色器中](./vshader_frag.glsl)中仿照实验3.2将几个矩阵分开，与此同时改变[main.cpp](./main.cpp)中的display()函数
* 由于投影需要是黑色，可以改变[片元着色器](./fshader_frag.glsl)，传入一个coefficients参数，控制各种反射的分量，如果是投影，直接让各个分量为0。
* 此时还是看不到:
    * 视角不对，参照**实验3.1**，改变视角
    * 经过调整，发现阴影在球的中央，此时需要
      * 将投影面设为-1 
      * 为了将投影面设为-1，需要先将物体向y方向移动一格，再投影，最后再向-y方向移动一格
## 遇到的问题
1. 新定义了一个plane发现没用，在init中添加进Buffer中，display中glGraw*有问题<br>
原因：是因为glDrawElements以buffer为绑定对象，偏移量是以子节为单位，最好使用sizeof()运算符
```c++
    glDrawElements(
        GL_TRIANGLES,
        int(2 * 3),
        GL_UNSIGNED_INT,
        (void *)(mesh->f().size() * sizeof(vec3i)) // 注意此处是以byte做单位，最好使用sizeof()
	);
```
2. 由于影子与投影矩形再同一个平面，会产生不稳定的图案，最好将矩形下移0.1个单位

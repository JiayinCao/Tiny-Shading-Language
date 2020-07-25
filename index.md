# Tiny-Shading-Language

TSL ( Tiny Shading Language ) is my own shading language designed for my offline renderer [**SORT**](http://sort-renderer.com/).

The goal of TSL is to provide shader programming ability to my open source ray tracer project. 
Although it is specifically designed for my own renderer, this programming language can totally be used in any other CPU based ray tracing project.

# How it works
TSL works in a very similar way with [OSL](https://github.com/imageworks/OpenShadingLanguage), but with some changes and simplification in it.

There are two type of shader templates in TSL, shader unit template and shader group template. Shader unit template is the very basics of shader definition, it is defined by a piece of source code with exactly one shader entry in it. Below is an example of a valid TSL shader that could be used to define a shader unit template

```
// a helper function that returns the base color
color get_base_color(){
    return color( 1.0f, 0.2f, 1.0f );
}

// the shader entry
shader closure_make(out closure o0){
    color diffuse = get_base_color();
    vector normal = vector( 0.0f, 1.0f, 0.0f );
    o0 = make_closure<lambert>( diffuse, normal );
}
```

There are a few things demonstrated in the above example
- Shader entry starts with the keyword of shader.
- There is a closure concept that is used to defer some operation to renderers, it is up to renderers to explain it.
- Each shader could define some helper function if needed.

A second type of shader template is called shader group template. Shader group template can't be defined directly with any source code, it is supposed to take shader unit templates in it and also the topology of these nodes, or put it in other words, the connections and exposed arguments.
This way, it matches perfectly well to modern material editors.

For example, following is a simple example of shader group template

```
  ----------------------------  Shader Group Template  --------------------------------
  |                                                                                   |
  |  ------ Base Color Shader ------                ------ Microfacet Shader ------   |
  |  |                             |                |                             |   |
  |  |                         color -------------->base_color              closure   |
  |  |                             |                |                             |   |
  |  -------------------------------                -------------------------------   |
  |                                                                                   |
  -------------------------------------------------------------------------------------
```

As we can see from the above graph, there are two shader unit templates in this shader group and the output of the base color shader is fed in the microfacet shader, which will eventually output a closure data for renderers to take.

Shader group template itself is also a shader unit template, which means that we can totally use a shader group template in another shader group template.
This is a very useful feature to help implementing groupping material nodes in material editors.

In order to execute the shader code, it is necessary to create a shader instance given a shader template, regardless whether it is a shader unit template or shader group template. Shader instance is the only shader execution unit in TSL, it caches a resolved raw function pointer for its host program to execute.

What is mentioned above is just a very brief introduction of how TSL works in a ray tracer. It doesn't covers all details in it. Hopefully, this could offer a quick big picture of how it works. In an ideal world, I should have written a language spec for it, but I don't have that much time.
For a more detailed tutorial, please check out the sample code in the source, which does demonstrates all that is needed to integrate TSL in a toy ray tracer.
Following is the image generated in the sample project,
![](https://github.com/JiayinCao/Tiny-Shading-Language/blob/master/gallery/tsl_sample.jpg?raw=true)

Apart from the sample code, my other project [SORT](http://sort-renderer.com/) has a way more practical integration solution in a ray tracer. It has more features used in it comparing with the sample code.

# Build TSL
Building TSL is fairly easy and straight forward. Before building, it is necessary to make sure C++ 17 is supported in your compiler.

The exact step of building is show below
- git clone https://github.com/JiayinCao/Tiny-Shading-Language.git tsl
- cd tsl
- make update_dep
- make

# Note
TSL is just a project that I did in my spare time in roughly four months, before when I had no experience of how a compiler works.
The original intention was just to replace OSL's features used in my renderer.

There are quite a few things to be noted
- It is very possible to crash the compiler
- Not every illegal way of shader programming will be indicated
- There are loads of features not well supported in TSL, like 3D texture sampling, mip-mapping

However, it does successfully replaces OSL with its own implementation in my renderer. I hope I have time to polish it in the future, but this will not be my focus in the near future.

# About Author

My name is Jiayin Cao. I’m currently working at Naughty Dog as a graphics engineer. Prior to joining Naughty Dog, I used to work at Ubisoft Singapore and NVIDIA Shanghai before. My primary interest covers the following areas,

- Real-time Rendering
- Offline Rendering
- GPGPU
- GPU architecture
- Mathematics

To those who wants to connect, for whatever reason, please contact me through the following media,

- [**Linkedin**](https://www.linkedin.com/in/caojiayin/)
- [**Twitter**](https://twitter.com/Jiayin_Cao)
- [**Email**](mailto:caojiayin1985@gmail.com)
- [**Tech Blog**](https://agraphicsguy.wordpress.com/)

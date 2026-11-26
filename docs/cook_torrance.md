# 第 18 讲 Cook–Torrance

实现采用 [FortuneRenderer 第 18 讲](https://github.com/woyaofacai/FortuneRenderer/tree/30b8777bd3820d89b8fe2dd131a60c8674dfa619/source/Chapter18) 的材质类、D/F/G 公式及 50/50 漫反射与 GGX 混合采样。材质构造参数顺序也与教程一致。

默认入口加载教程的 scene18.xml，使用金属球、木纹球和瓷砖墙地面。场景中的实际物体、光源和参数来自教程。上游额外声明的 M_CookTorrance_Ball3 没有被物体引用，且对应贴图未上传，本地移除了这段无效声明。其余贴图从同一版本仓库取得。

接入时同步了两处教程约定：三角形已经在构造时保存世界坐标，求交结果直接使用该坐标；材质采样返回零 PDF 时跳过后续反弹计算。

## XML

```xml
<Material>
    <Name>Metal</Name>
    <Type>CookTorrance</Type>
    <Albedo>1, 1, 1</Albedo>
    <AlbedoMap>Metal058C_1K-PNG_Color.png</AlbedoMap>
    <Roughness>1</Roughness>
    <RoughnessMap>Metal058C_1K-PNG_Roughness.png</RoughnessMap>
    <Metallic>1</Metallic>
    <MetallicMap>Metal058C_1K-PNG_Metalness.png</MetallicMap>
    <NormalMap>Metal058C_1K-PNG_NormalDX.png</NormalMap>
    <BumpScale>1</BumpScale>
</Material>
```

Roughness 默认 1，Metallic 默认 0，取值范围都是 [0, 1]。各贴图可省略，使用时与对应颜色或数值相乘。粗糙度最终限制在 [0.03, 1]。贴图路径支持相对于场景文件目录，以及该目录下的 Texture 子目录。

## 验证

```powershell
cmake -S . -B cmake-build-release -DMYRENDER_BUILD_TESTS=ON
cmake --build cmake-build-release
ctest --test-dir cmake-build-release --output-on-failure -R CookTorrance
```

检查包括独立公式、采样 PDF、贴图绑定、场景加载和非法参数。原有其他材质继续保留。

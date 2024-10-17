[![Build](https://github.com/platonvin/Circuli-Bellum/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/platonvin/Circuli-Bellum/actions/workflows/c-cpp.yml)
# Circulli Bellum 

some footage:

https://github.com/platonvin/Circuli-Bellum/blob/master/readme_content/video_1.mp4

**Circulli Bellum** is my clone of the game **"ROUNDS"**, but *somewhat* from scratch. The game uses [Box2D](https://github.com/erincatto/box2d) for physics and Vulkan for graphics (via [Lum-al](https://github.com/platonvin/lum-al))

Everything is drawn as a basic shape to use power of SDF's for "software" antalisaing (like MSAA but better)
 
To enhance viusals, there is bloom effect, separate shadow pass, and some simple chromatic aberration.

It’s not super optimized on the GPU-side (for that, check out my other project [Lum](https://github.com/platonvin/lum) ), but still runs pretty well
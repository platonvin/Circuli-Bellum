[![Build](https://github.com/platonvin/Circuli-Bellum/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/platonvin/Circuli-Bellum/actions/workflows/c-cpp.yml)

# Circulli Bellum 

<!-- simply ![footage](readme_content/cb.gif) does not work -->

https://github.com/user-attachments/assets/e60a025d-6315-42ad-a20d-7989150d5440

**Circulli Bellum** is my clone of the game **"ROUNDS"**, but *somewhat* from scratch. The game uses [Box2D](https://github.com/erincatto/box2d) for physics and Vulkan for graphics (via [Lum-al](https://github.com/platonvin/lum-al))

Everything is drawn as a basic shape to use power of SDF's for "software" antalisaing (like MSAA but better)
 
To enhance viusals, there is bloom effect, separate shadow pass, and some simple chromatic aberration.

It’s not super optimized on the GPU-side (for that, check out my other project [Lum](https://github.com/platonvin/lum) ), but still runs pretty well

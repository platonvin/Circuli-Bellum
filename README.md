[![Build](https://github.com/platonvin/Circuli-Bellum/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/platonvin/Circuli-Bellum/actions/workflows/c-cpp.yml)

# Circulli Bellum 

<!-- simply ![footage](readme_content/cb.gif) does not work -->

https://github.com/user-attachments/assets/e60a025d-6315-42ad-a20d-7989150d5440

**Circulli Bellum** is my clone of the game **"ROUNDS"**, but *somewhat* from scratch. The game uses [Box2D](https://github.com/erincatto/box2d) for physics and Vulkan for graphics (via [Lum-al](https://github.com/platonvin/lum-al))

Everything is drawn as a basic shape to use power of SDF's for "software" antalisaing (like MSAA but better)\
To enhance viusals, there is bloom effect, separate shadow pass, and some simple chromatic aberration.

It’s not super optimized on the GPU-side (for that, check out my other project [Lum](https://github.com/platonvin/lum) ), but still runs pretty well

All entities are stored in double-linked lists with only one pointer dereference and accessed by physics engine via curtom RTTI system (i prefer custom RTTI over standard C++ inheritance + override).\
At some point new(malloc) became a bottlneck but it was crutch-fixed with arena allocator.\
Currently, game suffers from giant structures accessed randomly (aka no cache coherence), but this will not be fixed. 
ECS (aka SoA) is what solves all these problems, but whole point was to learn how games are made commonly

```mermaid
flowchart TD
%% Nodes
    Frame["Frame"]
    
    MainPass["Main Pass"]
    ShadowPass["Shadow Pass"]
    BloomPass["Bloom Pass"]
    ApplyPass["Apply Pass"]
    
    BloomDownsample["Bloom Downsample"]
    BloomUpsample["Bloom Upsample"]
    
    ShadowApply["Shadow Apply"]
    Present[["Present"]]

%% Frame dependency
    Frame --> MainPass

%% Shadow and Bloom dependencies start from MainPass
    MainPass --> ShadowPass
    MainPass --> BloomPass

    %% Bloom Pass steps
    BloomPass --> BloomDownsample
    BloomDownsample --> BloomUpsample

    %% Shadow Pass steps
    ShadowPass --> ShadowApply

%% Apply Pass takes results from Bloom and Shadow
    BloomUpsample --> ApplyPass
    ShadowApply --> ApplyPass

%% Final output to Present
    ApplyPass --> Present

%% Styling
    style Frame color:#1E1A1D, fill:#AFAAB9, stroke:#1B2A41

    style MainPass color:#1E1A1D, fill:#7CA975, stroke:#1B2A41
    style ShadowPass color:#1E1A1D, fill:#7CA975, stroke:#1B2A41
    style BloomPass color:#1E1A1D, fill:#83C5B1, stroke:#1B2A41
    style BloomDownsample color:#1E1A1D, fill:#83C5BE, stroke:#1B2A41
    style BloomUpsample color:#1E1A1D, fill:#83C5BE, stroke:#1B2A41
    style ShadowApply color:#1E1A1D, fill:#7CA975, stroke:#1B2A41
    style ApplyPass color:#1E1A1D, fill:#83C5BE, stroke:#1B2A41
    style Present color:#1E1A1D, fill:#AFAAB9, stroke:#1B2A41
```

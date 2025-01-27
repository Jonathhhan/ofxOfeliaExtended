# Platform Support

- - - -

# Platform Compatibility
ofxImGui both implements a custom oF backend (similar to Jvcleave's original implementation) and a "native imgui backend". The custom oF backend is a very simple integration of basic ImGui functionality. It's very well integrated within OF but doesn't feature lots of advanced imgui features. On the opposite, the native GLFW backend provides all ImGui features but has to intercept the Glfw callbacks set by OpenFrameworks then forward them again.

For both backends, MacOs, Windows and Linux have been tested, but some more specific platforms remain to be tested; if you do so, please report back your findings.

*Notes on Rpi support: Some combinations of Rpi and oF versions won't provide all GLSL versions. Anyways, it's recommended to use the full KMS driver rather then the Legacy Broadcom one (very low FPS), but they both work. Tested with Raspbian Stretch. Also, if you start your application with a minimal desktop environment (using `startx ./bin/ofApp`), the imgui viewport features do not work correctly.* 

- - - -

# Backend support table

By default, ofxImGui uses the GLFW engine when possible, which gives the best user experience, falling back to the OpenFrameworks backend which is supported on most ofApp setups but provides slightly less ImGui functionality. You can force ofxImGui to use the OpenFrameworks one by defining `OFXIMGUI_FORCE_OF_BACKEND`.

One tiny disadvantage of the Glfw backend is that multiwindow-together-with-viewports support might break on ImGui updates. There are automatic update scripts, but they might not work in the future. Without the custom modifications, it will work fine but you'll have to choose between multiwindow or viewports, knowing that you can configure ImGui to never merge viewport windows with the host window.

| ofxImGui backend      | Viewports | Custom mouse cursors | Docking | Gamepads| GL SL | GL ES | Vulkan | Multiple ofxAppWindows | Automatic Contexts |
|-----------------------|-----------|----------------------|---------|---------|-------|-------|--------|------------------------|--------------------|
|  EngineGLFW           | [x]       | [x]*                 | [x]     | [x]     | [x]   | [x]   | Maybe  | [x]^                   | [x]!               |
|  EngineOpenFrameworks | [ ]       | [ ]                  | [x]     | [ ]     | [x]   | [x]   | Maybe  | [x]^                   | [x]!               |

- __*__ Partial support, by default not all cursors are enabled, see [Updating GLFW](./Developpers.md#Improve-ofxImGui-s-backend-bindings).
- __^__ One Context per ofAppWindow (isolated mode): No inter-communication between the GUIs (cross-docking won't work).  
  (_EngineGLFW backend is slightly modified for supporting multiple glfw contexts_)  
  Hopefully DearImGui will introduce something to handle «[multiple host viewports](https://github.com/ocornut/imgui/issues/3012)».  
  Please note that using **ofxImGui in multiwindow OpenFrameworks applications** works, but keep in mind that this might break with future ImGui updates.
- __!__ A singleton class ensures ensures the creation of the ImGui Context within openFrameworks. If multiple source files setup ofxImGui, the first sets up normally (as a master), the following ones as slaves, both still being able to draw to the same gui context. This can be useful when using ofxImGui from multiple ofxAddons.

- - - -

# GLSL support table

| OS      | OpenGL 2.x    | OpenGL 3.x    | OpenGL 4.x    | GL ES 1.0      | GL ES 2       | GL ES 3       |
|---------|---------------|---------------|---------------|----------------|---------------|---------------|
| Windows | Yes           | Yes           | Yes           | Unknown        | Unknown       | Unknown       |
| Mac OsX | Yes           | Yes           | Yes           |  *Unavailable* | *Unavailable* | *Unavailable* |
| Linux   | Yes           | Yes           | Yes           | Yes            | Yes           | Should        |
| Rpi3    | Should        | Unknown       | Unknown       | Yes            | Yes           | Yes           |
| Rpi4    | Unknown       | Should        | Should        | Should         | Should        | Should        |
| iOS     | *Unavailable* | *Unavailable* | *Unavailable* | Should         | Should        | Should        |

*Note: This support table does not take into account software emulated support for graphics APIs.*  
*Note: GL ES 1 (the fixed pipeline ES shading language) is not natively supported by the native DearImGui backend, but it works with [some dirty hacks](src/gles1CompatibilityHacks.h).*

- - - -

# oF & ImGui support table

New ImGui versions bring changes and new API features, sometimes depreciations.  
Versions are tagged in the git repo.

| ofxImGui version  | ImGui version | Tested oF version |
|------------------:|---------------|-------------------|
| ofxImGui 1.91.0   | 1.91.0*       | 0.11 -> 0.12.0 |
| ofxImGui 1.90.0   | 1.90.0*       | 0.11 -> 0.12.0 |
| ofxImGui 1.89.2   | 1.89.2*       | 0.11 -> 0.11.2 |
| ofxImGui 1.82     | 1.82*         | 0.11 -> 0.11.2 |
| ofxImGui 1.79     | 1.79*         | 0.11.1         |
| ofxImGui 1.75     | 1.75          | 0.11.x         |
| ofxImGui 1.62     | 1.62          | 0.10.x         | 
| ofxImGui 1.50     | 1.50 WIP      | 0.10.x         |
| ofxImGui 1.49     | 1.49          | 0.10.x         |
| ofxImGui 1.47     | 1.47          | 0.10.x         |
| ofxImGui 0.90     | 1.45 WIP      | 0.9.x          |

__*__ Uses the native ImGui backend, offering pop-out-windows (viewports), docking, gamepad control, and more.

- - - -

# GLFW version

The GLFW backend is bound to the GLFW version provided by Openframeworks, which is mostly an old version and even contains some issues related to an embedded pre-release version. Except on Linux where GLFW is not embedded within OpenFrameworks and it depends on your distro. 

If you use the GLFW version embedded by OpenFrameworks, the biggest drawback is that not all cursors are available, making the GUI a little less intuitive.  
Also, the viewport user experience is way better with an updated GLFW.  
On OSX, updating GLFW will also show the (cached) window content while resizing your ofAppWindows, which is quite a nice improvement.

*These support tables are an indication and might be incomplete.*

| Openframeworks | GLFW         | Source |
|----------------|--------------|--------|
| 0.11.0         | pre-3.3.0    | [Link](https://github.com/openframeworks/apothecary/blob/14c55b173c4110f05668089617b5a28ab7d110ce/apothecary/formulas/glfw.sh) |
| 0.11.1         | 3.3.0-stable | [Link](https://github.com/openframeworks/apothecary/commit/68a0ec866341a8487d5c555311f3d5975bd62436) |
| 0.11.2         | pre-3.3.0    | [Link](https://github.com/openframeworks/apothecary/pull/197) |
| 0.11.2_master  | 3.3.7        | [Link](https://github.com/openframeworks/apothecary/pull/225) | 
| 0.12.0         | 3.3.8        | [Link](https://github.com/openframeworks/apothecary/commit/bdc421bd28e8b433747759154f29a206d7cc9e41) |
| 0.12.x         | 3.4          | [Link](https://github.com/openframeworks/apothecary/commit/27b80288fc0e83ebad475b9ee94d042319bf0e3c) |

ImGui restrictions by GLFW version. [Source](https://github.com/ocornut/imgui/blob/v1.91.0-docking/backends/imgui_impl_glfw.cpp#L118-L145).  

| **GLFW**  | **Missing ImGui Features**   | 
|-----------|------------------------------|
| 3.1       | Keyboard input bugs, no window focusing, no window ordering + all below |
| 3.2       | Uses old gamepad API + all below |
| pre-3.3.0 | No resizing cursors, no deny cursor, window hovered, window alpha, multi-monitor-dpi + all below |
| 3.3.0     | No monitor workarea, no mouse pass-trough + all below |
| 3.3.8     | None |
| 3.4       | None |

oF 0.12.0 now bundles GLFW 3.3.8 which is OK with latest ImGui, but you can always update it to 3.4.
See [Configure.md](./Configure#update-glfw) for updating GLFW.


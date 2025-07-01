# L-Systems Animated

**Bringing L-systems to life with interactive animation**

This project visualizes L-systems using damped oscillators with level-wise parameters, compiled to WebAssembly for smooth real-time animation in the browser.



## 🌱 Try it Live

👉 [Test the L-System Viewer here](https://jaimelopezgarcia.github.io/lsystemsanimated/index.html)


![L-System Animation Demo](https://jaimelopezgarcia.github.io/lsystemsanimated/lsystem_animated.gif)

## Example Usage

```html
<canvas id="main-canvas" width="800" height="600"></canvas>
  <script src="https://jaimelopezgarcia.github.io/lsystemsanimated/lsystemsanimated.js"></script>
  <script src="https://jaimelopezgarcia.github.io/lsystemsanimated/lsystems_mod.js"></script>
<script>
Module.onRuntimeInitialized = function() {
  var F_value = "FF";
  var X_value = "F[+X]F[-X]+X";
  var iterations = 5;
  runLSystemAnimation("main-canvas", F_value, X_value,iterations);
};
</script>

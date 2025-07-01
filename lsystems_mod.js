(function(global){
  function Scaler2D() {
    this.scale = 1;
    this.offset = [0, 0];
  }
  Scaler2D.prototype.fit = function(floatArray, canvasWidth, canvasHeight) {
    if (floatArray.length < 2) return;
    let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
    for (let i = 0; i + 1 < floatArray.length; i += 2) {
      const x = floatArray[i], y = floatArray[i + 1];
      minX = Math.min(minX, x);
      maxX = Math.max(maxX, x);
      minY = Math.min(minY, y);
      maxY = Math.max(maxY, y);
    }
    const span = Math.max(maxX - minX, maxY - minY);
    this.scale = 0.75 * Math.min(canvasWidth, canvasHeight) / span;
    this.offset = [floatArray[0], floatArray[1]];
  };
  Scaler2D.prototype.forward = function(x, y) {
    return [
      (x - this.offset[0]) * this.scale + this.offset[0],
      (y - this.offset[1]) * this.scale + this.offset[1]
    ];
  };
  Scaler2D.prototype.inverse = function(cx, cy) {
    return [
      (cx - this.offset[0]) / this.scale + this.offset[0],
      (cy - this.offset[1]) / this.scale + this.offset[1]
    ];
  };

  global.runLSystemAnimation = function(canvasId, F_value, X_value, iterations) {
    function startWhenReady() {
      const create   = Module.cwrap('lsystem_physics_create','number',['string','string','string','number']);
      const step     = Module.cwrap('lsystem_physics_step'  ,'number',['number','number','number','number','number','number','number']);
      const destroy  = Module.cwrap('lsystem_physics_destroy','void',['number']);
      const canvas = document.getElementById(canvasId);
      const ctx    = canvas.getContext('2d');
      const W = canvas.width, H = canvas.height;
      let lsysPtr = create("F\nX", F_value + "\n" + X_value, "X", iterations);
      let outSizePtr = Module._malloc(4);
      let scaler = new Scaler2D();
      let firstFrame = true;
      let mouseX = 0, mouseY = 0;
      let animationId = null;
      canvas.addEventListener('mousemove', e => {
        const r = canvas.getBoundingClientRect();
        mouseX = e.clientX - r.left;
        mouseY = e.clientY - r.top;
      });
      function drawFrame() {
        let [mx, my] = scaler.inverse(mouseX, mouseY);
        const bufPtr = step(lsysPtr, outSizePtr, mx, my, 0.1, 100.0, 10.0);
        const outFloats = Module.HEAP32[outSizePtr >> 2];
        if (!bufPtr || outFloats < 4) {
          animationId = requestAnimationFrame(drawFrame);
          return;
        }
        const floatArray = new Float32Array(Module.HEAPF32.buffer, bufPtr, outFloats);
        if (firstFrame) {
          firstFrame = false;
          scaler.fit(floatArray, W, H);
        }
        ctx.clearRect(0, 0, W, H);
        ctx.strokeStyle = 'green';
        ctx.beginPath();
        for (let i = 0; i + 3 < outFloats; i += 4) {
          const [x1, y1] = scaler.forward(floatArray[i], floatArray[i + 1]);
          const [x2, y2] = scaler.forward(floatArray[i + 2], floatArray[i + 3]);
          ctx.moveTo(x1, y1);
          ctx.lineTo(x2, y2);
        }
        ctx.stroke();
        if (Module._free_float_buffer) Module._free_float_buffer(bufPtr);
        else if (Module._free) Module._free(bufPtr);
        animationId = requestAnimationFrame(drawFrame);
      }
      drawFrame();
    }
    if (global.Module && global.Module.onRuntimeInitialized) {
      // If already loaded, run immediately
      if (global.Module.calledRun) {
        startWhenReady();
      } else {
        global.Module.onRuntimeInitialized = startWhenReady;
      }
    } else {
      // If Module not present, wait for it to appear
      var check = setInterval(function() {
        if (global.Module && global.Module.onRuntimeInitialized) {
          clearInterval(check);
          if (global.Module.calledRun) {
            startWhenReady();
          } else {
            global.Module.onRuntimeInitialized = startWhenReady;
          }
        }
      }, 50);
    }
  };
})(window);

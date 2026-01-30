// Define Module BEFORE loading demo.js
var Module = {
  onRuntimeInitialized() {
	start();
  }
};

let playerTexture = null;
let playerTextureReady = false;

function loadPlayerTexture() {
  const img = new Image();
  img.src = "assets/player.png";
  img.onload = () => {
	playerTexture = img;
	playerTextureReady = true;
  };
  img.onerror = () => {
	playerTextureReady = false;
  };
}

function loadSkyTexture(setSkyTexture) {
  const skyImg = new Image();
  skyImg.src = "assets/pixel-sky.jpg";
  skyImg.onload = () => {
	const tmp = document.createElement("canvas");
	tmp.width = skyImg.width;
	tmp.height = skyImg.height;

	const tctx = tmp.getContext("2d");
	tctx.drawImage(skyImg, 0, 0);

	const imgData = tctx.getImageData(0, 0, skyImg.width, skyImg.height);
	const size = imgData.data.length;
	const ptr = Module._malloc(size);
	try {
	  const heap = Module.HEAPU8 || HEAPU8;
	  heap.set(imgData.data, ptr);
	  setSkyTexture(ptr, skyImg.width, skyImg.height);
	} catch (err) {
	  console.error("Failed to upload sky texture:", err);
	}
  };
  skyImg.onerror = () => {
	console.warn("Failed to load pixel-sky.jpg");
  };
}

function loadGroundTexture(setGroundTexture) {
	const groundImg = new Image();
	groundImg.src = "assets/ground.png";
	groundImg.onload = () => {
	  const tmp = document.createElement("canvas");
	  tmp.width = groundImg.width;
	  tmp.height = groundImg.height;
  
	  const tctx = tmp.getContext("2d");
	  tctx.drawImage(groundImg, 0, 0);
  
	  const imgData = tctx.getImageData(0, 0, groundImg.width, groundImg.height);
	  const size = imgData.data.length;
	  const ptr = Module._malloc(size);
	  try {
		const heap = Module.HEAPU8 || HEAPU8;
		heap.set(imgData.data, ptr);
		setGroundTexture(ptr, groundImg.width, groundImg.height);
	  } catch (err) {
		console.error("Failed to upload ground texture:", err);
	  }
	};
	groundImg.onerror = () => {
	  console.warn("Failed to load ground texture");
	};
  }

function loadObstacleTexture(setObstacleTexture) {
  const obstacleImg = new Image();
  obstacleImg.src = "assets/obstacle.png";
  obstacleImg.onload = () => {
	const tmp = document.createElement("canvas");
	tmp.width = obstacleImg.width;
	tmp.height = obstacleImg.height;

	const tctx = tmp.getContext("2d");
	tctx.drawImage(obstacleImg, 0, 0);

	const imgData = tctx.getImageData(0, 0, obstacleImg.width, obstacleImg.height);
	const size = imgData.data.length;
	const ptr = Module._malloc(size);
	try {
	  const heap = Module.HEAPU8 || HEAPU8;
	  heap.set(imgData.data, ptr);
	  setObstacleTexture(ptr, obstacleImg.width, obstacleImg.height);
	} catch (err) {
	  console.error("Failed to upload obstacle texture:", err);
	}
  };
  obstacleImg.onerror = () => {
	console.warn("Failed to load obstacle.png");
  };
}
  

function start() {
  console.log("WASM ready");
  loadPlayerTexture();

  const canvas = document.getElementById("game");
  const ctx = canvas.getContext("2d", { alpha: false });
  const scoreEl = document.getElementById("score");
  const gameOverEl = document.getElementById("game-over-screen");
  const highScoreEl = document.getElementById("high-score");

  // MUST match canvas width/height
  const W = canvas.width;
  const H = canvas.height;

  const img = ctx.createImageData(W, H);

  const gameStep = Module.cwrap("game_step", null, ["number"]);
  const getFramebuffer = Module.cwrap("get_framebuffer", "number");

  const setColor = Module.cwrap("set_color", null, ["number", "number", "number", "number"]);
  const growRect = Module.cwrap("grow_rect", null, []);
  const shrinkRect = Module.cwrap("shrink_rect", null, []);

  const moveRect = Module.cwrap("move_rect", null, ["number", "number"]);


  const playerIsUp = Module.cwrap("player_up", null, []);
  const playerIsDown = Module.cwrap("player_down", null, []);

  const resetGame = Module.cwrap("reset_game", null, []);
  const getGameScore = Module.cwrap("get_game_score", "number", []);
  const getGameOver = Module.cwrap("get_game_over", "number", []);
  const getPlayerX = Module.cwrap("get_player_x", "number", []);
  const getPlayerY = Module.cwrap("get_player_y", "number", []);
  const getPlayerSize = Module.cwrap("get_player_size", "number", []);
  const setSkyTexture = Module.cwrap("set_sky_texture", null, ["number", "number", "number"]);
  const setGroundTexture = Module.cwrap("set_ground_texture", null, ["number", "number", "number"]);
  const setObstacleTexture = Module.cwrap("set_obstacle_texture", null, ["number", "number", "number"]);

  loadSkyTexture(setSkyTexture);
  loadGroundTexture(setGroundTexture);
  loadObstacleTexture(setObstacleTexture);


  let last = performance.now();
  let lastScore = null;
  let lastGameOver = null;
  let highScore = 0;

  // keys that are pressed right now
  const keys = new Set();

  // this is the main loop that is called 60 times per second
  function loop(now) {
	// Clamp dt so tab-switch doesn't explode your movement
	let dt = (now - last) / 1000.0;
	last = now;
	if (dt > 0.05) dt = 0.05;

	// this is the step size for the movement of the rect
  const step = 200 * dt;
  if (joyX !== 0 || joyY !== 0) {
	moveRect(joyX * step, joyY * step);
  }

	if (keys.has("ArrowLeft")) moveRect(-step, 0);
	if (keys.has("ArrowRight")) moveRect(step, 0);
	// if (keys.has("ArrowUp")) moveRect(0, -step);
	// if (keys.has("ArrowDown")) moveRect(0, step);

	if (keys.has("ArrowUp")) playerIsUp();
	if (keys.has("ArrowDown")) playerIsDown();

	// if (keys.has("ArrowUp")) playerJump();

	// this is the function that is called to update the game state
	gameStep(dt);

	const ptr = getFramebuffer();
	const heap = Module.HEAPU8 || HEAPU8;
	const pixels = new Uint8ClampedArray(
		heap.buffer,
		ptr,
		img.data.length
	);

	img.data.set(pixels);
	ctx.putImageData(img, 0, 0);
	if (playerTextureReady) {
	  const px = getPlayerX();
	  const py = getPlayerY();
	  const ps = getPlayerSize();
	  ctx.imageSmoothingEnabled = false;
	  ctx.drawImage(playerTexture, px, py, ps, ps);
	}
	if (scoreEl) {
	  const score = getGameScore();
	  if (score !== lastScore) {
		scoreEl.textContent = String(score);
		lastScore = score;
	  }
	  if (highScoreEl && score > highScore) {
		highScore = score;
		highScoreEl.textContent = String(highScore);
	  }
	}
	if (gameOverEl) {
	  const isGameOver = getGameOver() === 1;
	  if (isGameOver !== lastGameOver) {
		gameOverEl.style.display = isGameOver ? "block" : "none";
		lastGameOver = isGameOver;
	  }
	}

	requestAnimationFrame(loop);
  }

  setColor(255, 0, 0, 255);

  function bindActionButton(button, action) {
	let repeatTimeout = null;
	let repeatInterval = null;
	const fire = () => {
	  if (action === "grow") playerIsUp();
	  if (action === "shrink") playerIsDown();
	  if (action === "reset") resetGame();
	};
	const clearTimers = () => {
	  if (repeatTimeout) {
		clearTimeout(repeatTimeout);
		repeatTimeout = null;
	  }
	  if (repeatInterval) {
		clearInterval(repeatInterval);
		repeatInterval = null;
	  }
	};
	const onDown = (e) => {
	  e.preventDefault();
	  fire();
	  clearTimers();
	  repeatTimeout = setTimeout(() => {
		fire();
		repeatInterval = setInterval(fire, 80);
	  }, 250);
	};
	const onUp = (e) => {
	  e.preventDefault();
	  clearTimers();
	};

	button.addEventListener("pointerdown", onDown);
	button.addEventListener("pointerup", onUp);
	button.addEventListener("pointerleave", onUp);
	button.addEventListener("pointercancel", onUp);

  }

  document.querySelectorAll("[data-action]").forEach((btn) => {
	bindActionButton(btn, btn.getAttribute("data-action"));
  });

  // joystick is the joystick element
  const joystick = document.getElementById("joystick");
  const stick = document.getElementById("stick");

  // float values for the joystick
  let joyX = 0;
  let joyY = 0;
  let joyPointerId = null;


  function updateStick(clientX, clientY) {
	const rect = joystick.getBoundingClientRect();
	const cx = rect.left + rect.width / 2;
	const cy = rect.top + rect.height / 2;
	const dx = clientX - cx;
	const dy = clientY - cy;
	const maxRadius = (rect.width / 2) - (stick.offsetWidth / 2);
	const dist = Math.hypot(dx, dy);
	const clamped = dist > maxRadius ? maxRadius / dist : 1;
	const mx = dx * clamped;
	const my = dy * clamped;
	stick.style.transform = `translate(${mx}px, ${my}px)`;
	joyX = mx / maxRadius;
	joyY = my / maxRadius;
  }


  function resetStick() {
	stick.style.transform = "translate(0, 0)";
	joyX = 0;
	joyY = 0;
  }

  
  joystick.addEventListener("pointerdown", (e) => {
	e.preventDefault();
	joyPointerId = e.pointerId;
	joystick.setPointerCapture(e.pointerId);
	updateStick(e.clientX, e.clientY);
  });

  joystick.addEventListener("pointermove", (e) => {
	if (joyPointerId !== e.pointerId) return;
	e.preventDefault();
	updateStick(e.clientX, e.clientY);
  });

  joystick.addEventListener("pointerup", (e) => {
	if (joyPointerId !== e.pointerId) return;
	e.preventDefault();
	joystick.releasePointerCapture(e.pointerId);
	joyPointerId = null;
	resetStick();
  });

  joystick.addEventListener("pointercancel", (e) => {
	if (joyPointerId !== e.pointerId) return;
	e.preventDefault();
	joyPointerId = null;
	resetStick();
  });

  document.addEventListener("keydown", (e) => {
	if (e.code === "Equal") {
	  e.preventDefault();
	  growRect();
	  return;
	}
	if (e.code === "Minus") {
	  e.preventDefault();
	  shrinkRect();
	  return;
	}
	if (e.code.startsWith("Arrow")) {
	  e.preventDefault();
	  keys.add(e.code);
	}
  });

  document.addEventListener("keyup", (e) => {
	if (e.code.startsWith("Arrow")) {
	  e.preventDefault();
	  keys.delete(e.code);
	}
  });

  requestAnimationFrame(loop);
}

let lastTouchEnd = 0;

document.addEventListener('touchend', e => {
  const now = Date.now();
  if (now - lastTouchEnd <= 300) {
    e.preventDefault();
  }
  lastTouchEnd = now;
}, { passive: false });

document.addEventListener('gesturestart', e => e.preventDefault());
document.addEventListener('gesturechange', e => e.preventDefault());
document.addEventListener('gestureend', e => e.preventDefault());
let skyTexture1Path = "assets/wolt/sky.png";
let skyTexture2Path = "assets/wolt/skyline.png";
let skyTexture3Path = "assets/wolt/clouds.png";

let groundTexturePath = "assets/wolt/ground.webp";
let groundTexture2Path = "assets/wolt/bushes.webp";
let obstacleTexturePath = "assets/wolt/paper_bag.webp";
let bonusTexturePath = "assets/wolt/cake.webp";

let playerTexturePath = "assets/wolt/deer-sprite.webp";

let jumpSoundPath = "assets/sounds/jump.m4a";
let bonusSoundPath = "assets/sounds/bite.m4a";
let gameOverSoundPath = "assets/sounds/game-over.m4a";

let joystickSensitivy = 200;

// Define Module BEFORE loading demo.js
var Module = {
  onRuntimeInitialized() {
	start();
  }
};

let skyTexture1 = null;
let skyTexture2 = null;
let skyTexture1Ready = false;
let skyTexture2Ready = false; 

let playerTexture = null;
let playerTextureReady = false;
let audioContext = null;
let audioBuffers = {};
let audioUnlocked = false;

function initAudio() {
  if (!audioContext) {
	const AudioCtx = window.AudioContext || window.webkitAudioContext;
	if (!AudioCtx) return;
	audioContext = new AudioCtx();
  }
  if (audioContext.state === "suspended") {
	audioContext.resume().catch(() => {});
  }
}

// function loadPlayerTexture() {
//   const img = new Image();
//   img.src = playerTexturePath;
//   img.onload = () => {
// 	playerTexture = img;
// 	playerTextureReady = true;
//   };
//   img.onerror = () => {
// 	playerTextureReady = false;
//   };
// }

function loadSound(path, key) {
	return new Promise((resolve, reject) => {
	  initAudio();
	  if (!audioContext) {
		resolve(); // audio optional
		return;
	  }
  
	  fetch(path)
		.then(res => res.arrayBuffer())
		.then(data => audioContext.decodeAudioData(data))
		.then(buffer => {
		  audioBuffers[key] = buffer;
		  resolve();
		})
		.catch(err => {
		  console.warn("Audio load failed:", path);
		  resolve(); // don't block game on audio
		});
	});
  }
  

function playSound(key) {
  if (!audioContext) return;
  const buffer = audioBuffers[key];
  if (!buffer) return;
  try {
	const source = audioContext.createBufferSource();
	source.buffer = buffer;
	source.connect(audioContext.destination);
	source.start(0);
  } catch (err) {
	// ignore playback errors
  }
}

function playUnlockSound() {
  if (!audioContext) return;
  try {
	const buffer = audioContext.createBuffer(1, 1, audioContext.sampleRate);
	const source = audioContext.createBufferSource();
	source.buffer = buffer;
	source.connect(audioContext.destination);
	source.start(0);
  } catch (err) {
	// ignore playback errors
  }
}

function playJumpSound() {
  playSound("jump");
}

function playBonusSound() {
  playSound("bonus");
}

function playGameOverSound() {
  playSound("gameOver");
}

function loadTexture(texturePath, functionToCall) {
	return new Promise((resolve, reject) => {
	  const img = new Image();
	  img.src = texturePath;
  
	  img.onload = () => {
		const tmp = document.createElement("canvas");
		tmp.width = img.width;
		tmp.height = img.height;
  
		const tctx = tmp.getContext("2d");
		tctx.drawImage(img, 0, 0);
  
		const imgData = tctx.getImageData(0, 0, img.width, img.height);
		const size = imgData.data.length;
		const ptr = Module._malloc(size);
  
		try {
		  const heap = Module.HEAPU8 || HEAPU8;
		  heap.set(imgData.data, ptr);
		  functionToCall(ptr, img.width, img.height);
		  resolve();
		} catch (err) {
		  reject(err);
		}
	  };
  
	  img.onerror = () => reject(new Error(`Failed to load ${texturePath}`));
	});
  }



async function start() {
  console.log("WASM ready");

  loadSound(jumpSoundPath, "jump");
  loadSound(bonusSoundPath, "bonus");
  loadSound(gameOverSoundPath, "gameOver");

  const enableAudio = () => initAudio();
  const unlockAudio = () => {
	if (audioUnlocked) return;
	initAudio();
	if (!audioContext) return;
	audioContext.resume().then(() => {
	  playUnlockSound();
	  playSound("jump");
	  audioUnlocked = true;
	}).catch(() => {});
  };

  document.addEventListener("pointerdown", unlockAudio, { once: true });
  document.addEventListener("touchstart", unlockAudio, { once: true, passive: true });
  document.addEventListener("keydown", enableAudio, { once: true });

  const canvas = document.getElementById("game");
  const ctx = canvas.getContext("2d", { alpha: false });
  const loadingEl = document.getElementById("loading");
  const scoreEl = document.getElementById("score");
  const gameOverEl = document.getElementById("game-over-screen");
  const highScoreEl = document.getElementById("high-score");
  const speedValueEl = document.getElementById("speed-value");

  const W = canvas.width;
  const H = canvas.height;

  const img = ctx.createImageData(W, H);

  // functions to call from the WASM module
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
  const getGameStatus = Module.cwrap("get_game_status", "number", []);
  const getSpeed = Module.cwrap("get_speed", "number", []);
  const getBonusCollected = Module.cwrap("get_bonus_collected", "number", []);
  const getJumpTriggered = Module.cwrap("get_jump_triggered", "number", []);
  const getGameOverTriggered = Module.cwrap("get_game_over_triggered", "number", []);

  const getPlayerX = Module.cwrap("get_player_x", "number", []);
  const getPlayerY = Module.cwrap("get_player_y", "number", []);
  const getPlayerSize = Module.cwrap("get_player_size", "number", []);
  const setPlayerTexture = Module.cwrap("set_player_texture", null, ["number", "number", "number"]);

  const setSkyTexture1 = Module.cwrap("set_sky_texture1", null, ["number", "number", "number"]);
  const setSkyTexture2 = Module.cwrap("set_sky_texture2", null, ["number", "number", "number"]);
  const setSkyTexture3 = Module.cwrap("set_sky_texture3", null, ["number", "number", "number"]);

  const setGroundTexture = Module.cwrap("set_ground_texture", null, ["number", "number", "number"]);
  const setGroundTexture2 = Module.cwrap("set_ground_texture2", null, ["number", "number", "number"]);

  const setObstacleTexture = Module.cwrap("set_obstacle_texture", null, ["number", "number", "number"]);
  const setBonusTexture = Module.cwrap("set_bonus_texture", null, ["number", "number", "number"]);

  console.log("Loading assets...");
  if (loadingEl) {
	loadingEl.style.display = "flex";
  }
  const assetPromises = [
	loadTexture(playerTexturePath, setPlayerTexture),
	loadTexture(skyTexture1Path, setSkyTexture1),
	loadTexture(skyTexture2Path, setSkyTexture2),
	loadTexture(skyTexture3Path, setSkyTexture3),
	loadTexture(groundTexturePath, setGroundTexture),
	loadTexture(groundTexture2Path, setGroundTexture2),
	loadTexture(obstacleTexturePath, setObstacleTexture),
	loadTexture(bonusTexturePath, setBonusTexture),

	loadSound("assets/jump.m4a", "jump"),
	loadSound("assets/bite.m4a", "bonus"),
	loadSound("assets/game-over.m4a", "gameOver"),
  ];
  await Promise.all(assetPromises);
  console.log("Assets ready");
  if (loadingEl) {
	loadingEl.style.display = "none";
  }

  let last = performance.now();
  let lastScore = null;
  let lastGameStatus = null;
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
  const step = joystickSensitivy * dt;
  if (joyX !== 0 || joyY !== 0) {
	moveRect(joyX * step, joyY * step);
  }

	if (keys.has("ArrowLeft")) moveRect(-step, 0);
	if (keys.has("ArrowRight")) moveRect(step, 0);

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
		const status = getGameStatus();

		if (status !== lastGameStatus) {
		  if (status === 2) {
			gameOverEl.style.display = "block";
			gameOverEl.textContent = "GAME WON";
		  } 
		  else if (status === -1) {
			gameOverEl.style.display = "block";
			gameOverEl.textContent = "GAME OVER";
		  } 
		  else {
			gameOverEl.style.display = "none";
		  }
		
		  lastGameStatus = status;
		}
	  }

	if (speedValueEl) {
	  const speed = getSpeed() + 1.0;
	  speedValueEl.textContent = speed.toFixed(0);
	}

	if (getBonusCollected() === 1) {
	  playBonusSound();
	}
	if (getJumpTriggered() === 1) {
	  playJumpSound();
	}
	if (getGameOverTriggered() === 1) {
	  playGameOverSound();
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
	  unlockAudio();
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
	if (e.code === "Space") {
	  e.preventDefault();
	  resetGame();
	  return;
	}
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
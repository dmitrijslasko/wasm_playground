export function loadTexture(texturePath, Module, function_to_call) {
	console.log("Uploading", path, img.width, img.height);
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
		function_to_call(ptr, img.width, img.height);
	  } catch (err) {
		console.error("Failed to upload sky texture:", err);
	  }
	};
	img.onerror = () => {
	  console.warn(`Failed to load ${texturePath}`);
	};
  }
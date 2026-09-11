(function () {
	const MAP_PATH = "/error-pages/invalid-requests-map.html";

	const button = document.querySelector("a.button");
	if (!button) return;

	let cameFromMap = false;
	if (document.referrer) {
		try {
			const from = new URL(document.referrer, window.location.href);
			cameFromMap =
				from.origin === window.location.origin && from.pathname === MAP_PATH;
		} catch (e) {
			cameFromMap = false;
		}
	}

	if (cameFromMap) {
		button.href = MAP_PATH;
		button.textContent = "Back to the list";
	}
})();

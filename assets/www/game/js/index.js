const VALID_USERNAME = /^[A-Za-z0-9_-]{1,20}$/;

const form = document.querySelector("form");
const usernameInput = document.querySelector('input[name="username"]');
const termsInput = document.querySelector('input[name="terms"]');
const notice = document.getElementById("login-error");

const MESSAGES = {
	username: "Usernames must be 1-20 characters: letters, digits, - or _.",
	terms: "You have to accept the terms before waddling in.",
	method: "Please submit the form to log in.",
};

function showError(text) {
	if (!notice) {
		alert(text);
		return;
	}
	notice.textContent = text;
	notice.hidden = false;
}

const reason = new URLSearchParams(window.location.search).get("error");
if (reason) showError(MESSAGES[reason] || "Something went wrong. Please try again.");

if (form) {
	form.addEventListener("submit", (event) => {
		const name = (usernameInput ? usernameInput.value : "").trim();

		if (!VALID_USERNAME.test(name)) {
			event.preventDefault();
			showError(MESSAGES.username);
			if (usernameInput) usernameInput.focus();
			return;
		}
		if (termsInput && !termsInput.checked) {
			event.preventDefault();
			showError(MESSAGES.terms);
			return;
		}
	});
}

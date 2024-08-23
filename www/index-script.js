/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 */

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {

	// Attach button handlers
	document.getElementById("reboot").onclick = function() {
		PUTRequest('/reboot', 'Success, rebooting!');
	};

	document.getElementById("reset").onclick = function() {
		if (confirm("Reset WiFi settings?") == true) {
			PUTRequest('/reset', 'Success, rebooting!');
		}
	};
	
	document.getElementById("settime").onclick = function() {
		POSTRequest('/setTime', "Time set", 'time', '{"time":' + Math.floor(new Date().getTime() / 1000) + ',"offset":' + (0 - new Date().getTimezoneOffset() * 60) + '}');
	};

});

// Send a POST request with an optional key/value data pair as data_name and payload (payload can be a single value or a JSON string)
function POSTRequest(path, successMessage, data_name = "", payload = "") {
	let xhr = new XMLHttpRequest(), data = new FormData();
	if (data_name != "") {
		data.append(data_name, payload);
	}
	xhr.open('POST', path);
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			document.getElementById('message').innerHTML = successMessage;
		}
	};
	xhr.send(data); 
}

// Send a PUT request
function PUTRequest(path, successMessage) {
	let xhr = new XMLHttpRequest();
	xhr.open('PUT', path);
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			document.getElementById('message').innerHTML = successMessage;
		}
	};
	xhr.send();
}
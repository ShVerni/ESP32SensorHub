/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 */

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
    GETRequest(addSensors, "/sensors/");
    GETRequest(addReceivers, "/signals/");

});

// Adds all connected sensors to the page
function addSensors(sensors) {
	const holder = document.getElementById("sensors");
	if (sensors.sensors.length === 0) {
		holder.innerHTML ='<p>No active sensors</p>';
	} else {
		holder.innerHTML = '<div class="button-container">\r\n';
		sensors.sensors.forEach(sensor => {
			holder.innerHTML += '<button class="def-button" onclick="loadDevice(true,\'' + sensor.description.name + '\',' +  sensor.positionID +')">' + sensor.description.name + '</button>';
		});
	}
}

// Adds all connected signal receivers to the page
function addReceivers(receivers) {
	const holder = document.getElementById("receivers");
	if (receivers.receivers.length === 0) {
		holder.innerHTML = '<p>No active receivers</p>';
	} else {
		holder.innerHTML = '<div class="button-container">\r\n';
		receivers.receivers.forEach(receiver => {
			holder.innerHTML += '<button class="def-button" onclick="loadDevice(false,\'' + receiver.description.name + '\',' +  receiver.positionID +')">' + receiver.description.name + '</button>';
		});
	}
}

// Adds a signal receiver to the page for configuring
function loadDevice(isSensor, name, posID) {
	const holder = document.getElementById("device");
	holder.dataset.posid = posID;
	holder.dataset.sensor = isSensor;
	device.data
	holder.innerHTML = '<h2>' + name + '</h2>';
	let path = "/signals/config";
	let dataName = "receiver";
	if (isSensor) {
		path = "/sensors/config";
		dataName = "sensor";
	}
	data = {};
	data[dataName] = posID;
	GETRequest(addDeviceConfig, path, data);
}

// Adds a device configuration to the page
function addDeviceConfig(device) {
	if (Object.keys(device).length === 0 ) {
		holder.innerHTML += '<p>No configurable options</p>';
	} else {
		const holder = document.getElementById("device");
		let type = "text";
		let step = 1;
		for (opt in device) {
			let name = opt.replace(" ", "_");
			if (typeof(device[opt]) === "number") {
				type = "number";
				if (!Number.isInteger(device[opt])) {
					step = 0.01;
				}
				} else {

			}
			holder.innerHTML += '<label for="' + name + '">' + opt + '</label>';
			holder.innerHTML += '<input class="stacked-input" type="' + type + '" name="' + name + '" step="' + step + '" value="' + device[opt] + '">';
		}
		holder.innerHTML += '<div class="button-container"><button class="def-button" onclick="updateDeviceConfig(' + holder.dataset.sensor + ',' + holder.dataset.posid + ')">Update Config</button></div>';
	}
}

// Parses and updates config for device
function updateDeviceConfig(isSensor, posID) {
	const inputs = document.querySelectorAll('#device input');
	let new_config = {};
	Array.from(inputs).forEach((input) => {
		if (input.type === "number") {
			new_config[input.name] = input.valueAsNumber;
		} else {
			new_config[input.name] = input.value;
		}
	});
	console.log(new_config);
	let path = "/signals/config";
	let dataName = "receiver";
	if (isSensor) {
		path = "/sensors/config";
		dataName = "sensor";
	}
	let data = {};
	data[dataName] = posID;
	data['config'] = JSON.stringify(new_config);
	POSTRequest(path, "Device config updated!", data);
}

// Send a POST request with an optional object of key/value pairs for parameters
function POSTRequest(path, successMessage, params = {}) {
	document.getElementById('message').innerHTML = "";
	let xhr = new XMLHttpRequest(), data = new FormData();
	if (Object.keys(params).length !== 0 ) {
		for (param in params) {
			data.append(param, params[param]);
		}
	}
	xhr.open('POST', path);
	xhr.onload = function() {
		if (this.status !== 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			document.getElementById('message').innerHTML = successMessage;
		}
	};
	xhr.send(data); 
}

// Send a GET request with an optional object of key value pairs for parameters
// Returns the response to the callback provided
function GETRequest(callback, path, params = {}) {
    let xhr = new XMLHttpRequest();
	xhr.responseType = 'json';
    if (Object.keys(params).length !== 0 ) {
		let first = true
		path += "?";
		for (param in params) {
			if (first) {
				first = false;
			} else {
				path += "&";
			}
			path += param + "=" + params[param];
		}
	}
    xhr.open('GET', path);
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			let response = xhr.response;
			console.log(response);
			callback(response);
		}
	};
	xhr.send();
}
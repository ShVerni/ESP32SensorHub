/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 */

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
    GETRequest("/sensors/", addSensors);
    GETRequest("/signals/", addReceivers);

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
	GETRequest(path, addDeviceConfig, data);
}

// Adds a device configuration to the page
function addDeviceConfig(device) {
	const holder = document.getElementById("device");
	if (Object.keys(device).length === 0 ) {
		holder.innerHTML += '<p>No configurable options</p>';
	} else {
		let type = "text";
		let step = 1;
		let additionalAttrb = "";
		for (opt in device) {
			let name = opt.replace(" ", "_");
			if (typeof(device[opt]) === "number") {
				type = "number";
				if (!Number.isInteger(device[opt])) {
					step = 0.01;
				}
			} else if (typeof(device[opt]) === "boolean") {
				type = "checkbox";
				if (device[opt] === true) {
					additionalAttrb= "checked";
				}
			}
			holder.innerHTML += '<label for="' + name + '">' + opt + '</label>';
			holder.innerHTML += '<input class="stacked-input" type="' + type + '" name="' + name + '" step="' + step + '" value="' + device[opt] + '" ' + additionalAttrb +'>';
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
		} else if (input.type === "checkbox") {
			new_config[input.name] = input.checked;
		} 
		else {
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
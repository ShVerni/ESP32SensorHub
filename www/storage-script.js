/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 */

// File upload handler
let uprog = {
	hBar: null,
	hPercent: null,
	hFile: null,
	init: () => {
		uprog.hBar = document.getElementById('up-bar');
		uprog.hPercent = document.getElementById('up-percent');
		uprog.hFile = document.getElementById('up-file');
		uprog.hFile.disabled = false;
	},
	update: (percent) => {
		let Percent = percent + '%';
		uprog.hBar.style.width = Percent;
		uprog.hPercent.innerHTML = Percent;
		if (Percent == '100%') { uprog.hFile.disabled = false; }
	},
	upload: (path) => {
		if (uprog.hFile.files.length == 0) {
			return;
		}
		let file = uprog.hFile.files[0];
		uprog.hFile.disabled = true;
		uprog.hFile.value = '';
		let xhr = new XMLHttpRequest(), data = new FormData();
		data.append('upfile', file);
		xhr.open('POST', '/upload-file');
		xhr.setRequestHeader('FILE_UPLOAD_PATH', path);
		let percent = 0;
		xhr.upload.onloadstart = () => { uprog.update(0); };
		xhr.upload.onloadend = () => { uprog.update(100); };
		xhr.upload.onprogress = (evt) => {
			percent = Math.ceil((evt.loaded / evt.total) * 100);
			uprog.update(percent);
		};
		xhr.onload = function() {
			if (this.status == 507) {
				document.getElementById('message').innerHTML = "Not enough free storage for file!";
			} else if (this.status != 201) {
				document.getElementById('message').innerHTML = this.response;
			} else {
				uprog.update(100);
				document.getElementById('message').innerHTML = 'File uploaded!';
				updateFileList();
				getFreeStorage();
			}
		};
		xhr.send(data);
	}
};

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	// Initialize the upload handler
	uprog.init();

	// Get server info
	getFreeStorage();
	updateFileList();

	// Attach button handlers
	document.getElementById("up-www").onclick = function() {
		uprog.upload('/www');
	};
});

// Update the list of files displayed on this page
function updateFileList() {
	document.getElementById("file-list").innerHTML = "";
	getFileList("/", 5);
}

// Delete file
function deleteFile(file) {
	let name = file.getAttribute('data-name');
	if (confirm("Delete " + name + "?")) {
		let xhr = new XMLHttpRequest(), data = new FormData();
		data.append('path', name);
		xhr.open('POST', '/delete');
		xhr.onload = function() {
			if (this.status != 200) {
				document.getElementById('message').innerHTML = this.response;
			} else {
				document.getElementById('message').innerHTML = 'File deleted!';
				file.parentNode.remove();
				getFreeStorage();
			}
		};
		xhr.send(data);
	}
}

// Get free storage space
function getFreeStorage() {
	let xhr = new XMLHttpRequest();
	xhr.responseType = 'json';
	xhr.open('GET', '/freeSpace');
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			let response = xhr.response;
			console.log(response);
			if (response != null) {
				let space = document.getElementById("freespace");
				space.innerHTML = 'Free space: ' + response.space + ' bytes';
			}
		}
	};
	xhr.send();
}

// Get list of files, add to DOM
function getFileList(path, depth = 0) {
	let xhr = new XMLHttpRequest();
	xhr.responseType = 'json';
	xhr.open('GET', '/list?path=' + path + "&depth=" + depth);
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			let response = xhr.response;
			console.log(response);
			if (response != null) {
				let list = document.getElementById("file-list");
				for (let i = 0; i < response.files.length; i++)
				{
					list.innerHTML += `
					<tr class="file">
						<td>` + response.files[i] + `</td>
						<td class="download"><a href="/download?path=` + response.files[i] + `">Download</a>
						<td class="delete" onclick="deleteFile(this)" data-name="` + response.files[i] + `">Delete</td>
					</tr>`;
				}
			}
		}
	};
	xhr.send();
}
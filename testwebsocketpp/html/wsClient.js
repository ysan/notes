class wsClient {
	constructor (url) {
		this.url = url;
		this.webSocket = undefined;
		this.list = [];
		this.timerId;
	}

	open (_src) {
		console.log("open");
		clearInterval(this.timerId);

		if (this.webSocket === undefined) {
			;;
		} else if (this.webSocket.readyState === undefined) {
			;;
		} else if (this.webSocket.readyState !== WebSocket.CLOSED && this.webSocket.readyState !== WebSocket.CLOSING) {
			console.log ("WebSocket is already opened.");
			return;
		}

		console.log(`debug ${this.url}`);
		this.webSocket = new WebSocket (this.url);
		
		let self = this;
		
		this.webSocket.onopen = function () {
			console.log("ws.onopen");
		};

		this.webSocket.onmessage = function (event) {
//			console.log("ws.onmessage");

			let num = self.list.length;
			if (num >= 100) {
				self.list.shift();
			}
			self.list.push(event.data);

			let _log = "";
			for (let i = 0; i < self.list.length; ++ i) {
				_log += self.list[i] + "<br>";
			}

			let logw = document.getElementById("logw");
			logw.innerHTML = _log;

			logw.scrollBy(0, logw.clientHeight);
		};

		this.webSocket.onclose = function () {
			console.log("ws.onclose");
			self.timerId = setInterval (function(){self.open(self);}, 3000, self);
		};

		this.webSocket.onerror = function () {
			console.log("ws.onerror");
			try { 
				self.webSocket.close();
			} catch (e) {
				console.log("ws.close error");
			}
		};
	}

	close () {
		try {
			if (this.webSocket === undefined) {
				;;
			} else if (this.webSocket.readyState === undefined) {
				;;
			} else if (this.webSocket.readyState !== WebSocket.CONNECTING && this.webSocket.readyState !== WebSocket.CLOSED) {
				this.webSocket.close();
			}
		} catch (e) {
			console.log("ws.close error");
		}
	};

	getState () {
		if (this.webSocket == null) {
			return -1;
		}
		if (this.webSocket.readyState == null) {
			return -1;
		}
		return this.webSocket.readyState;
	}
}
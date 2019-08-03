/**
 * Send a request for the websocket port
 * Open a websocket to that port
 */
function startWebsocket() {
  webSocketStatus              = document.getElementById("websocket-status");
  webSocket                    = new WebSocket(webSocketAddress);
  webSocket.onreadystatechange = webSocketReadyState;
  webSocket.onerror            = function(event) {
    console.log(event.code);
    webSocketStatus.innerHTML = "Could not connect to " + webSocketAddress;
  };
}

/**
 * On webSocket ready state change, check for open connection
 */
function webSocketReadyState() {
  if (this.readyState == WebSocket.OPEN) {
    webSocketStatus.innerHTML = "Connected to port " + webSocketPort;
    console.log("Connected");
  } else if (this.readyState == WebSocket.CONNECTING) {
    webSocketStatus.innerHTML = "Connecting to port " + webSocketPort;
    console.log("Connecting");
  } else if (this.readyState == WebSocket.CLOSING) {
    webSocketStatus.innerHTML = "Closing port " + webSocketPort;
    console.log("Closing");
  } else if (this.readyState == WebSocket.CLOSED) {
    webSocketStatus.innerHTML = "Closed port " + webSocketPort;
    console.log("Closed");
  }
}

window.onload = startWebsocket;

var webSocket = null;
var webSocketAddress =
    "ws://" + window.location.hostname + ":" + window.location.port;
var webSocketStatus = null;
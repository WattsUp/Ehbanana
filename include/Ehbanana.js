/**
 * On receiving a message from the WebSocket
 * @param {WebSocket event} event
 */
function webSocketMessage(event) {
  console.log(event.data);
}

/**
 * Send a request for the websocket port
 * Open a websocket to that port
 */
function startWebsocket() {
  webSocketStatus     = document.getElementById("websocket-status");
  webSocket           = new WebSocket(webSocketAddress);
  webSocket.onmessage = webSocketMessage;
  webSocket.onclose   = function(event) {
    webSocketStatus.innerHTML = "Closed connection to " + webSocketAddress;
  };
  webSocket.onopen = function(event) {
    webSocket.send("Hello WebSocket");
    webSocketStatus.innerHTML = "Opened connection to " + webSocketAddress;
  };
  webSocket.onerror = function(event) {
    console.log(event.code);
    webSocketStatus.innerHTML = "Could not connect to " + webSocketAddress;
  };
}

window.onload = startWebsocket;

var webSocket = null;
var webSocketAddress =
    "ws://" + window.location.hostname + ":" + window.location.port;
var webSocketStatus = null;
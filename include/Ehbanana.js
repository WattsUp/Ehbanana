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
    string = "Hello WebSocket|";
    // for(var i = 0; i < Math.pow(2, 16); i++)
    //   string +="Hello WebSocket|";
    webSocket.send(string);
    webSocketStatus.innerHTML = "Opened connection to " + webSocketAddress;
  };
  webSocket.onerror = function(event) {
    console.log(event.code);
    webSocketStatus.innerHTML = "Could not connect to " + webSocketAddress;
  };
}

window.addEventListener("load", startWebsocket);

var webSocket = null;
var webSocketAddress =
    "ws://" + window.location.hostname + ":" + window.location.port;
var webSocketStatus = null;
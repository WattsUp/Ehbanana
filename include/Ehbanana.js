/**
 * Send a request for the websocket port
 * Open a websocket to that port
 */
function startWebsocket() {
  webSocketStatus            = document.getElementById("websocket-status");
  var xmlHTTP                = new XMLHttpRequest();
  xmlHTTP.onreadystatechange = xmlHTTPReadyState;
  xmlHTTP.open("GET", "~/websocket", true);
  xmlHTTP.send();
}

/**
 * On XML HTTP request ready state change, check for successful completion then
 * open a websocket
 */
function xmlHTTPReadyState() {
  if (this.readyState == XMLHttpRequest.DONE) {
    if (this.status == 200) {
      webSocketPort             = this.responseText;
      webSocketStatus.innerHTML = "Found GUI port at " + webSocketPort;

      webSocket = new WebSocket("ws://127.0.0.1:" + webSocketPort, "Ehbanana");
      webSocket.onreadystatechange = webSocketReadyState;
      webSocket.onerror            = function() {
        webSocketStatus.innerHTML =
            "Could not connect to port " + webSocketPort;
      };
    } else
      webSocketStatus.innerHTML = "Cannot find GUI port ERROR: " + this.status;
  }
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

var webSocket       = null;
var webSocketPort   = null;
var webSocketStatus = null;
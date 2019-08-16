/**
 * On receiving a message from the WebSocket
 * @param {WebSocket event} event
 */
function webSocketMessage(event) {
  jsonEvent = JSON.parse(event.data);
  obj       = document.getElementById(jsonEvent.name);
  switch (obj.nodeName) {
    case "DIV":
      obj.innerHTML = jsonEvent.value;
      break;
    case "INPUT":
      obj.value = jsonEvent.value;
      break;
    default:
      console.log(obj.nodeName);
  }
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
    webSocketStatus.innerHTML = "Opened connection to " + webSocketAddress;
    attachListeners();
  };
  webSocket.onerror = function(event) {
    webSocketStatus.innerHTML = "Could not connect to " + webSocketAddress;
  };
}

/**
 * Listen to an input event and send the results to the WebSocket
 * @param {InputEvent} event
 */
function listenerInput(event) {
  var jsonEvent = {name: event.target.name, value: event.target.value};
  if (event.target.type == "checkbox")
    jsonEvent.checked = event.target.checked;
  else if (event.target.type == "file") {
    jsonEvent.fileSize = event.target.files[0].size;
    webSocket.send(JSON.stringify(jsonEvent));
    webSocket.send(event.target.files[0]); // Sent as binary
    return;
  }
  webSocket.send(JSON.stringify(jsonEvent));
}

/**
 * Add a listener to each element with "eb-*" class
 */
function attachListeners() {
  var elementsInput = document.getElementsByClassName("eb-input");
  for (var i = 0; i < elementsInput.length; i++) {
    if (elementsInput[i].getAttribute("name") == null) {
      if (elementsInput[i].getAttribute("id") == null) {
        console.log("No name nor id for eb-input:", elementsInput[i]);
        continue;
      }
      elementsInput[i].setAttribute("name", elementsInput[i].id);
    }
    if (elementsInput[i].getAttribute("type") == "button")
      elementsInput[i].addEventListener("click", listenerInput);
    else
      elementsInput[i].addEventListener("input", listenerInput);
  }
}

window.addEventListener("load", startWebsocket);

var webSocket = null;
var webSocketAddress =
    "ws://" + window.location.hostname + ":" + window.location.port;
var webSocketStatus = null;